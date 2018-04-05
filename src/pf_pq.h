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

struct pf_pq {
    struct accum_node *heap;
    size_t size;
    size_t alloc;
};

struct pf_pq *
pf_pq_create(size_t size);

void
pf_pq_destroy(struct pf_pq *pq);

int
pf_pq_enqueue(struct pf_pq *pq, char *const val, const double prio);

int
pf_pq_find(const struct pf_pq *pq, struct accum_node *acc_node);

int
pf_pq_delete(struct pf_pq *pq);

int
pf_pq_dequeue(struct pf_pq *pq, struct accum_node *acc_node);

int
pf_pq_cmp(const struct pf_pq *pq, int a, int b);

void
pf_pq_swap(const struct pf_pq *pq, int a, int b);

size_t
pf_pq_size(const struct pf_pq *pq);

#endif /* PF_PQ_H */
