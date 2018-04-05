/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef RBC_ACCUM_H
#define RBC_ACCUM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

struct rbc_accum {
    size_t capacity;
    size_t size;
    struct accum_node *data;
    int topic;
    bool is_set;
};

struct accum_node {
    char *docno;
    double val;
    bool is_set;
    size_t count;
};

struct rbc_accum *
rbc_accum_create(size_t capacity);

void
rbc_accum_free(struct rbc_accum *htable);

unsigned long
rbc_accum_update(struct rbc_accum **htable, const char *val, double score);

struct rbc_accum *
rbc_accum_rehash(struct rbc_accum *htable);

#endif /* RBC_ACCUM_H */
