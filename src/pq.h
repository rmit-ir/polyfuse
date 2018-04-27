/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PF_PQ_H
#define PF_PQ_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pf_accum.h"
#include "util.h"

struct pq {
    struct accum_node *heap;
    size_t size;
    size_t alloc;
};

struct pq *
pq_create(size_t size);

void
pq_destroy(struct pq *pq);

int
pq_insert(struct pq *pq, char *const val, const long double prio,
    const size_t count);

int
pq_remove(struct pq *pq, struct accum_node *res);

int
pq_min(const struct pq *pq, struct accum_node *res);

int
pq_delete(struct pq *pq);

int
pq_cmp(const struct pq *pq, size_t a, size_t b);

void
pq_swap(const struct pq *pq, size_t a, size_t b);

size_t
pq_size(const struct pq *pq);

#endif /* PF_PQ_H */
