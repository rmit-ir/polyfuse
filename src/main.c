/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fusetype.h"
#include "polyfuse.h"
#include "trec.h"

#define DEFAULT_DEPTH 1000
#define FBORDA "borda"
#define FCOMBANZ "combanz"
#define FCOMBMAX "combmax"
#define FCOMBMED "combmed"
#define FCOMBMIN "combmin"
#define FCOMBMNZ "combmnz"
#define FCOMBSUM "combsum"
#define FISR "isr"
#define FLOGISR "logisr"
#define FRBC "rbc"
#define FRRF "rrf"
#define CMDSTR_LEN 8
#define AVAILCMDS                                                          \
    "  borda, combanz, combmax, combmed, combmin, combmnz, combsum, isr, " \
    "logisr, rbc, rrf",

static enum fusetype cmd = TNONE;
static char cmd_str[CMDSTR_LEN] = {0};
static long double phi = 0.8;
static long rrf_k = 60;
static enum trec_norm fnorm = TNORM_NONE;
static size_t depth = DEFAULT_DEPTH;
static bool prevent_ties = false;
char *runid = NULL;
// the indices must align with `enum fusetype` entries
const char *default_runid[] = {
    "", /* TNONE */
    "polyfuse-combanz",
    "polyfuse-combmax",
    "polyfuse-combmed",
    "polyfuse-combmin",
    "polyfuse-combmnz",
    "polyfuse-combsum",
    "polyfuse-rbc",
    "polyfuse-rrf",
    "polyfuse-borda",
    "polyfuse-isr",
    "polyfuse-logisr",
};

static int
parse_opt(int argc, char **argv);

static void
usage(void);

static void
present_args();

static enum trec_norm
strtonorm(const char *s);

static FILE *
next_file(int argc, char **argv)
{
    FILE *fp = NULL;

    if (argc < 1) {
        return fp;
    }
    if (!(fp = fopen(argv[optind++], "r"))) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    return fp;
}

static bool
is_score_based(enum fusetype type)
{
    switch (type) {
    case TCOMBANZ:
    case TCOMBMAX:
    case TCOMBMED:
    case TCOMBMIN:
    case TCOMBMNZ:
    case TCOMBSUM:
        return true;
    default:
        return false;
    }
}

#define RUNFILE_WEIGHT_INIT_SZ 128
static double runfile_weights[RUNFILE_WEIGHT_INIT_SZ] = {0.0};
static size_t runfile_weights_len = 0;
/* struct runfile_weight { */
/*     double *ary; */
/*     size_t len; */
/*     size_t alloc; */
/* }; */

/* struct runfile_weight * */
/* runfile_weight_create() */
/* { */
/*     struct runfile_weight *weights = NULL; */

/*     weights = bmalloc(sizeof(struct runfile_weight)); */
/*     weights->ary = bmalloc(sizeof(double) * RUNFILE_WEIGHT_INIT_SZ); */
/*     weights->len = 0; */
/*     weights->alloc = RUNFILE_WEIGHT_INIT_SZ; */

/*     return weights; */
/* } */

int
main(int argc, char **argv)
{
    int left;
    bool first = true;
    FILE *fp;

    left = parse_opt(argc, argv);
    present_args();
    printf("%d left\n", left);

    size_t j = 0; /* runfile weight index */
    for (size_t i = left; (fp = next_file(i, argv)) != NULL; i--) {
        struct trec_run *r = trec_create();
        trec_read(r, fp);
        if (is_score_based(cmd)) {
            /*
             * Normalize score based fusion measures.
             */
            trec_normalize(r, fnorm);
        }
        if (first) {
            /*
             * All run files are assumed to have the same topics and are taken
             * from the first file given on the commandline.
             */
            pf_set_fusion(cmd);
            pf_set_rrf_k(rrf_k);
            pf_init(&r->topics);
            first = false;
        }

        pf_weight_alloc(phi, r->max_rank);
        pf_accumulate(r, runfile_weights[j++]);
        trec_destroy(r);
        fclose(fp);
    }

    pf_present(stdout, runid, depth, prevent_ties);
    pf_destory();
    free(runid);

    return 0;
}

/*
 * Parse commandline options.
 */
static int
parse_opt(int argc, char **argv)
{
    int ch;

    if (argc > 1) {
        if (0 == strncmp(argv[optind], "-h", 3)) {
            usage();
            exit(EXIT_SUCCESS);
        } else if (0 == strncmp(argv[optind], "-v", 3)) {
            printf("polyfuse %s\n", POLYFUSE_VERSION);
            exit(EXIT_SUCCESS);
        }

        if (0 == strncmp(argv[optind], FBORDA, strlen(argv[optind]))) {
            cmd = TBORDA;
            strncpy(cmd_str, FBORDA, CMDSTR_LEN);
        } else if (0 ==
                   strncmp(argv[optind], FCOMBANZ, strlen(argv[optind]))) {
            cmd = TCOMBANZ;
            strncpy(cmd_str, FCOMBANZ, CMDSTR_LEN);
        } else if (0 ==
                   strncmp(argv[optind], FCOMBMAX, strlen(argv[optind]))) {
            cmd = TCOMBMAX;
            strncpy(cmd_str, FCOMBMAX, CMDSTR_LEN);
        } else if (0 ==
                   strncmp(argv[optind], FCOMBMED, strlen(argv[optind]))) {
            cmd = TCOMBMED;
            strncpy(cmd_str, FCOMBMED, CMDSTR_LEN);
        } else if (0 ==
                   strncmp(argv[optind], FCOMBMIN, strlen(argv[optind]))) {
            cmd = TCOMBMIN;
            strncpy(cmd_str, FCOMBMIN, CMDSTR_LEN);
        } else if (0 ==
                   strncmp(argv[optind], FCOMBMNZ, strlen(argv[optind]))) {
            cmd = TCOMBMNZ;
            strncpy(cmd_str, FCOMBMNZ, CMDSTR_LEN);
        } else if (0 ==
                   strncmp(argv[optind], FCOMBSUM, strlen(argv[optind]))) {
            cmd = TCOMBSUM;
            strncpy(cmd_str, FCOMBSUM, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FISR, strlen(argv[optind]))) {
            cmd = TISR;
            strncpy(cmd_str, FISR, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FLOGISR, strlen(argv[optind]))) {
            cmd = TLOGISR;
            strncpy(cmd_str, FLOGISR, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FRBC, strlen(argv[optind]))) {
            cmd = TRBC;
            strncpy(cmd_str, FRBC, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FRRF, strlen(argv[optind]))) {
            cmd = TRRF;
            strncpy(cmd_str, FRRF, CMDSTR_LEN);
        } else {
            cmd = TNONE;
        }

        if (TNONE == cmd) {
            err_exit("unknown fusion command '%s'\n\navailable commands "
                     "are:\n" AVAILCMDS argv[optind]);
        }

        optind++;
    }

    char opt_str[10] = {0};
    if (TRBC == cmd) {
        strcpy(opt_str, "td:r:p:");
    } else if (TRRF == cmd) {
        strcpy(opt_str, "td:r:k:w:");
    } else if (is_score_based(cmd)) {
        strcpy(opt_str, "td:r:n:");
    } else {
        strcpy(opt_str, "td:r:");
    }

    while ((ch = getopt(argc, argv, opt_str)) != -1) {
        switch (ch) {
        case 't':
            prevent_ties = true;
            break;
        case 'd':
            depth = strtoul(optarg, NULL, 10);
            break;
        case 'r':
            runid = strdup(optarg);
            break;
        case 'k':
            rrf_k = strtol(optarg, NULL, 10);
            break;
        case 'w':
            runfile_weights[runfile_weights_len++] = strtod(optarg, NULL);
            /* printf("%s\n", optarg); */
            /* printf("double %f\n", runfile_weights[runfile_weights_len]); */
            break;
        case 'n':
            fnorm = strtonorm(optarg);
            if (TNORM_NONE == fnorm) {
                err_exit("unknown normalization '%s'\n\nvalid normalizations "
                         "are:\n minmax, sum, minsum, std",
                    optarg);
            }
            break;
        case 'p':
            phi = strtod(optarg, NULL);
            break;
        case '?':
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    argc -= optind;
    if (argc < 2) {
        usage();
        exit(EXIT_FAILURE);
    }

    /* rrf weights equal to runfiles? */
    assert(argc == (int) runfile_weights_len);

    if (!runid) {
        runid = strdup(default_runid[cmd]);
    }

    return argc;
}

/*
 * Display program usage.
 */
static void
usage(void)
{
    fprintf(stderr,
        "usage: polyfuse [-v] [-h] "
        "<fusion> [options] run1 run2 [run3 ...]\n"
        "\noptions:\n"
        "  -d depth     rank depth of output\n"
        "  -t           prevent ties\n"
        "  -h           display this message\n"
        "  -r runid     set run identifier\n"
        "  -v           display version and exit\n"
        "\nfusion commands:\n"
        "  borda        Borda count\n"
        "  combanz      CombANZ\n"
        "  combmax      CombMAX\n"
        "  combmed      CombMED\n"
        "  combmin      CombMIN\n"
        "  combmnz      CombMNZ\n"
        "  combsum      CombSUM\n"
        "  isr          Inverse square rank\n"
        "  logisr       Logarithmic inverse square rank\n"
        "  rbc          Rank-biased centroids\n"
        "  rrf          Recipocal rank fusion\n"
        "\nnormalization options:\n"
        "  minmax       min-max scaler\n"
        "  std          zero mean and unit variance\n"
        "  sum          sum normalization\n"
        "  minsum       min-sum scaler\n"
        "\nscore-based fusion options (combanz, ..., combsum):\n"
        "  -n norm      perform score normalization before fusion\n"
        "\nrbc options:\n"
        "  -p num       user persistence in the range (0.0,1.0)\n"
        "\nrrf options:\n"
        "  -k num       constant to control outlier rankings\n\n");
}

static void
present_args()
{
    fprintf(stderr, "# runid: %s\n", runid);
    fprintf(stderr, "# depth: %ld\n", depth);
    fprintf(stderr, "# fusion: %s\n", cmd_str);
    if (TRBC == cmd) {
        fprintf(stderr, "# phi: %Lf\n", phi);
    } else if (TRRF == cmd) {
        fprintf(stderr, "# k: %ld\n", rrf_k);
    } else if (is_score_based(cmd)) {
        fprintf(stderr, "# normalization: %s\n", trec_norm_str[fnorm]);
    }
}

static enum trec_norm
strtonorm(const char *s)
{
    const char *opts[] = {"minmax", "minsum", "sum", "std"};
    enum trec_norm norm = TNORM_NONE;

    if (strncmp(opts[0], s, strlen(opts[0])) == 0) {
        norm = TNORM_MINMAX;
    } else if (strncmp(opts[1], s, strlen(opts[1])) == 0) {
        norm = TNORM_MINSUM;
    } else if (strncmp(opts[2], s, strlen(opts[2])) == 0) {
        norm = TNORM_SUM;
    } else if (strncmp(opts[3], s, strlen(opts[3])) == 0) {
        norm = TNORM_ZMUV;
    }

    return norm;
}
