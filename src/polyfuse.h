/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PF_H
#define PF_H

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fusetype.h"
#include "pf_topic.h"
#include "pq.h"
#include "trec.h"

void
pf_weight_alloc(const long double phi, const size_t len);

void
pf_init(const struct trec_topic *topics);

void
pf_destory();

void
pf_accumulate(struct trec_run *r, double weight);

void
pf_set_fusion(const enum fusetype type);

void
pf_set_rrf_k(const long k);

long double
pf_score(size_t rank, size_t n, struct trec_entry *tentry);

void
pf_present(FILE *stream, const char *id, size_t depth, bool prevent_ties);

#endif /* RBC_H */
