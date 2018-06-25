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

enum accumtype { ACCUM_NONE, ACCUM_DBL, ACCUM_LIST };

struct ldbl_arr;

struct default_entry {
    char *docno;
    bool is_set;
};

struct dbl_entry {
    char *docno;
    bool is_set;
    long double val;
    size_t count;
};

struct list_entry {
    char *docno;
    bool is_set;
    struct ldbl_arr *ary;
};

/*
 * Base accumulator.
 */
struct accum {
    uint8_t type;
    size_t capacity;
    size_t size;
    int topic;
    bool is_set;
    struct default_entry *data;
};

/*
 * Long double accumulator.
 */
struct accum_dbl {
    uint8_t type;
    size_t capacity;
    size_t size;
    int topic;
    bool is_set;
    struct dbl_entry *data;
};

/*
 * List accumulator.
 */
struct accum_list {
    uint8_t type;
    size_t capacity;
    size_t size;
    int topic;
    bool is_set;
    struct list_entry *data;
};

struct accum *
accum_dbl_create(const size_t capacity);

void
accum_dbl_free(struct accum_dbl *htable);

unsigned long
accum_dbl_less(struct accum **htable, const char *docno, long double score);

unsigned long
accum_dbl_greater(struct accum **htable, const char *docno, long double score);

unsigned long
accum_dbl_update(struct accum **htable, const char *docno, long double score);

struct accum *
accum_list_create(const size_t capacity);

void
accum_list_free(struct accum_list *acc);

long double
accum_list_median(const struct list_entry *l);

unsigned long
accum_list_append(struct accum **htable, const char *docno, long double score);

#endif /* PF_ACCUM_H */
