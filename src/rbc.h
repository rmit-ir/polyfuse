/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef RBC_H
#define RBC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "rbc_pq.h"
#include "rbc_topic.h"
#include "trec.h"

void
rbc_init(const struct trec_topic *topics,
    const double phi, const size_t depth);

void
rbc_destory();

void
rbc_accumulate(struct trec_run *r);

void
rbc_present(FILE *stream, const char *id);

#endif /* RBC_H */
