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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fusetype.h"
#include "pf_pq.h"
#include "pf_topic.h"
#include "trec.h"

void
pf_weight_alloc(const double phi, const size_t len);

void
pf_init(const struct trec_topic *topics);

void
pf_destory();

void
pf_accumulate(struct trec_run *r);

void
pf_set_fusion(const enum fusetype type);

void
pf_set_rrf_k(const long k);

double
pf_score(size_t rank, struct trec_entry *tentry);

void
pf_present(FILE *stream, const char *id, size_t depth);

#endif /* RBC_H */