/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

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
#define FCOMBSUM "combsum"
#define FCOMBMNZ "combmnz"
#define FISR "isr"
#define FLOGISR "logisr"
#define FRBC "rbc"
#define FRRF "rrf"
#define CMDSTR_LEN 8
#define AVAILCMDS "  borda, combsum, combmnz, isr, logisr, rbc, rrf",

static enum fusetype cmd = TNONE;
static char cmd_str[CMDSTR_LEN] = {0};
static double phi = 0.8;
static long rrf_k = 60;
static size_t depth = DEFAULT_DEPTH;
char *runid = NULL;
const char *default_runid[] = {
    "", /* TNONE */
    "polyfuse-combsum", "polyfuse-rbc", "polyfuse-rrf", "polyfuse-combmnz",
    "polyfuse-borda", "polyfuse-isr", "polyfuse-logisr",
};

static int
parse_opt(int argc, char **argv);

static void
usage(void);

static void
present_args();

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

int
main(int argc, char **argv)
{
    int left;
    bool first = true;
    FILE *fp;

    left = parse_opt(argc, argv);
    present_args();

    for (size_t i = left; (fp = next_file(i, argv)) != NULL; i--) {
        struct trec_run *r = trec_create();
        trec_read(r, fp);
        if (first) {
            /*
             * All run files are assumed to have the same topics and are taken
             * from the first file given on the commandline.
             */
            pf_init(&r->topics);
            pf_set_fusion(cmd);
            pf_set_rrf_k(rrf_k);
            first = false;
        }

        pf_weight_alloc(phi, r->len);
        pf_accumulate(r);
        trec_destroy(r);
        fclose(fp);
    }

    pf_present(stdout, runid, depth);
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

        if (0 == strncmp(argv[optind], FCOMBSUM, strlen(argv[optind]))) {
            cmd = TCOMBSUM;
            strncpy(cmd_str, FCOMBSUM, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FRBC, strlen(argv[optind]))) {
            cmd = TRBC;
            strncpy(cmd_str, FRBC, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FRRF, strlen(argv[optind]))) {
            cmd = TRRF;
            strncpy(cmd_str, FRRF, CMDSTR_LEN);
        } else if (0 ==
                   strncmp(argv[optind], FCOMBMNZ, strlen(argv[optind]))) {
            cmd = TCOMBMNZ;
            strncpy(cmd_str, FCOMBMNZ, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FBORDA, strlen(argv[optind]))) {
            cmd = TBORDA;
            strncpy(cmd_str, FBORDA, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FISR, strlen(argv[optind]))) {
            cmd = TISR;
            strncpy(cmd_str, FISR, CMDSTR_LEN);
        } else if (0 == strncmp(argv[optind], FLOGISR, strlen(argv[optind]))) {
            cmd = TLOGISR;
            strncpy(cmd_str, FLOGISR, CMDSTR_LEN);
        } else {
            cmd = TNONE;
        }

        if (TNONE == cmd) {
            err_exit("unkown fusion command '%s'\n\navailable commands are:\n"
                     AVAILCMDS
                argv[optind]);
        }

        optind++;
    }

    char opt_str[7] = {0};
    if (TRBC == cmd) {
        strcpy(opt_str, "d:r:p:");
    } else if (TRRF == cmd) {
        strcpy(opt_str, "d:r:k:");
    } else {
        strcpy(opt_str, "d:r:");
    }

    while ((ch = getopt(argc, argv, opt_str)) != -1) {
        switch (ch) {
        case 'd':
            depth = strtoul(optarg, NULL, 10);
            break;
        case 'r':
            runid = strdup(optarg);
            break;
        case 'k':
            rrf_k = strtol(optarg, NULL, 10);
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

    if (!runid) {
        runid = strdup(default_runid[cmd]);
    }

    argc -= optind;
    if (argc < 2) {
        usage();
        exit(EXIT_FAILURE);
    }

    return argc;
}

/*
 * Display program usage.
 */
static void
usage(void)
{
    fprintf(stderr, "usage: polyfuse [-v] [-h] "
                    "<fusion> [options] run1 run2 [run3 ...]\n"
                    "\noptions:\n"
                    "  -d depth     rank depth to calculate\n"
                    "  -h           display this message\n"
                    "  -r runid     set run identifier\n"
                    "  -v           display version and exit\n"
                    "\nfusion commands:\n"
                    "  borda        Borda count\n"
                    "  combsum      CombSUM\n"
                    "  combmnz      CombMNZ\n"
                    "  isr          Inverse square rank\n"
                    "  logisr       Logarithmic inverse square rank\n"
                    "  rbc          Rank-biased centroids\n"
                    "  rrf          Recipocal rank fusion\n"
                    "\nrbc options:\n"
                    "  -p num       user persistence in the range [0.0,1.0]\n"
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
        fprintf(stderr, "# phi: %f\n", phi);
    } else if (TRRF == cmd) {
        fprintf(stderr, "# k: %ld\n", rrf_k);
    }
}
