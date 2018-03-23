/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef RBC_TOPIC_H
#define RBC_TOPIC_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "rbc_accum.h"
#include "util.h"

struct rbc_topic {
    size_t capacity;
    size_t size;
    struct rbc_accum **data;
};

struct rbc_topic *
rbc_topic_create(size_t capacity);

void
rbc_topic_free(struct rbc_topic *htable);

unsigned long
rbc_topic_insert(struct rbc_topic **htable, const int val);

struct rbc_accum **
rbc_topic_lookup(struct rbc_topic *htable, const int val);

struct rbc_topic *
rbc_topic_rehash(struct rbc_topic *htable);

#endif /* RBC_TOPIC_H */
