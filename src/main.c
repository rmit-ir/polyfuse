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

#include "rbc.h"
#include "trec.h"

#define DEFAULT_DEPTH 1000
#define RBC_RUNID "polyfuse-rbc"

static double phi = 0.8;
static size_t depth = DEFAULT_DEPTH;
char *runid = NULL;

static int
parse_opt(int argc, char **argv);

static void
usage(void);

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

    for (size_t i = left; (fp = next_file(i, argv)) != NULL; i--) {
        struct trec_run *r = trec_create();
        trec_read(r, fp);
        if (first) {
            rbc_init(&r->topics, phi, depth);
            first = false;
        }

        rbc_accumulate(r);
        trec_destroy(r);
        fclose(fp);
    }

    rbc_present(stdout, runid);
    rbc_destory();
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

    while ((ch = getopt(argc, argv, "hd:p:r:v")) != -1) {
        switch (ch) {
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
            break;
        case 'd':
            depth = strtoul(optarg, NULL, 10);
            break;
        case 'p':
            phi = strtod(optarg, NULL);
            break;
        case 'r':
            runid = strdup(optarg);
            break;
        case 'v':
            printf("polyfuse %s\n", POLYFUSE_VERSION);
            exit(EXIT_SUCCESS);
            break;
        case '?':
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    if (!runid) {
        runid = strdup(RBC_RUNID);
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
    fprintf(stderr, "Usage: polyfuse [option] run1 run2 [run3 ...]\n");
    fprintf(stderr, "\nOptions:\n"
                    "  -t type      Fusion algorithm to use. Options are:\n"
                    "               combsum|rbc|rrf\n"
                    "  -p num       User persistence in the range [0.0,1.0]\n"
                    "  -d depth     Rank depth to calculate\n"
                    "  -r runid     Set run identifier\n"
                    "  -h           Display this message\n"
                    "  -v           Display version and exit\n\n");
}
