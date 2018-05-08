/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TREC_H
#define TREC_H

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

enum trec_norm { TNORM_NONE, TNORM_MINMAX, TNORM_SUM, TNORM_ZMUV };
extern const char *trec_norm_str[];

struct trec_entry {
    int qid;
    char *docno;
    int rank;
    long double score;
    char *name;
};

struct trec_topic {
    int *ary;
    size_t len;
    size_t alloc;
};

struct trec_run {
    struct trec_entry *ary;
    size_t len;
    size_t alloc;
    struct trec_topic topics;
    size_t max_rank;
};

struct trec_run *
trec_create();

void
trec_destroy(struct trec_run *run);

void
trec_read(struct trec_run *r, FILE *fp);

void
trec_normalize(struct trec_run *r, enum trec_norm norm);

#endif /* TREC_H */
