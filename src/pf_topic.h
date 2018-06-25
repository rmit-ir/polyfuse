/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PF_TOPIC_H
#define PF_TOPIC_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "pf_accum.h"
#include "util.h"

struct pf_topic {
    size_t capacity;
    size_t size;
    struct accum **data;
};

void
enable_list_accumulator();

struct pf_topic *
pf_topic_create(size_t capacity);

void
pf_topic_free(struct pf_topic *htable);

unsigned long
pf_topic_insert(struct pf_topic **htable, const int val);

struct accum **
pf_topic_lookup(struct pf_topic *htable, const int val);

struct pf_topic *
pf_topic_rehash(struct pf_topic *htable);

#endif /* PF_TOPIC_H */
