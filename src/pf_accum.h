/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PF_ACCUM_H
#define PF_ACCUM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

struct pf_accum {
    size_t capacity;
    size_t size;
    struct accum_node *data;
    int topic;
    bool is_set;
};

struct accum_node {
    char *docno;
    long double val;
    bool is_set;
    size_t count;
};

struct pf_accum *
pf_accum_create(size_t capacity);

void
pf_accum_free(struct pf_accum *htable);

unsigned long
pf_accum_less(struct pf_accum **htable, const char *docno, long double score);

unsigned long
pf_accum_greater(
    struct pf_accum **htable, const char *docno, long double score);

unsigned long
pf_accum_update(
    struct pf_accum **htable, const char *docno, long double score);

struct pf_accum *
pf_accum_rehash(struct pf_accum *htable);

#endif /* PF_ACCUM_H */
