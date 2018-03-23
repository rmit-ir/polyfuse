/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef RBC_PQ_H
#define RBC_PQ_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "rbc_accum.h"
#include "util.h"

struct rbc_pq {
    struct accum_node *heap;
    size_t size;
    size_t alloc;
};

struct rbc_pq *
rbc_pq_create(size_t size);

void
rbc_pq_destroy(struct rbc_pq *pq);

int
rbc_pq_enqueue(struct rbc_pq *pq, char *const val, const double prio);

int
rbc_pq_find(const struct rbc_pq *pq, struct accum_node *acc_node);

int
rbc_pq_delete(struct rbc_pq *pq);

int
rbc_pq_dequeue(struct rbc_pq *pq, struct accum_node *acc_node);

int
rbc_pq_cmp(const struct rbc_pq *pq, int a, int b);

void
rbc_pq_swap(const struct rbc_pq *pq, int a, int b);

size_t
rbc_pq_size(const struct rbc_pq *pq);

#endif /* RBC_PQ_H */
