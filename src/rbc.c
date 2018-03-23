/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "rbc.h"

struct topic_list {
    int *ary;
    size_t size;
};

static double *weights = NULL;
static size_t weight_sz = 0;

static struct rbc_topic *topic_tab = NULL;
static struct topic_list qids = {NULL, 0};

void
rbc_init(const struct trec_topic *topics,
    const double phi, const size_t depth)
{
    // for now just assume the trec run files are the same length and have the
    // same topics

    weight_sz = depth;
    weights = (double *)bmalloc(sizeof(double) * weight_sz);
    double w = 1 - phi;
    for (size_t i = 0; i < weight_sz; i++) {
        weights[i] = w;
        w *= phi;
    }

    qids.ary = bmalloc(sizeof(int) * topics->len);
    qids.size = topics->len;
    memcpy(qids.ary, topics->ary, sizeof(int) * topics->len);

    topic_tab = rbc_topic_create(topics->len);
    for (size_t i = 0; i < topics->len; i++) {
        rbc_topic_insert(&topic_tab, topics->ary[i]);
    }
}

void
rbc_destory()
{
    free(weights);
    free(qids.ary);
    rbc_topic_free(topic_tab);
}

void
rbc_accumulate(struct trec_run *r)
{
    for (size_t i = 0; i < r->len; i++) {
        size_t rank = r->ary[i].rank - 1;
        if (rank < weight_sz) {
            double w = weights[rank];
            struct rbc_accum **curr;
            curr = rbc_topic_lookup(topic_tab, r->ary[i].qid);
            if (*curr) {
                rbc_accum_update(curr, r->ary[i].docno, w);
            }
        }
    }
}

void
rbc_present(FILE *stream, const char *id)
{
    for (size_t i = 0; i < qids.size; i++) {
        struct rbc_accum *curr;
        curr = *rbc_topic_lookup(topic_tab, qids.ary[i]);
        struct rbc_pq *pq = rbc_pq_create(weight_sz);
        // this is why we use linear probing
        for (size_t j = 0; j < curr->capacity; j++) {
            if (!curr->data[j].is_set) {
                continue;
            }
            rbc_pq_enqueue(pq, curr->data[j].docno, curr->data[j].val);
        }
        struct accum_node *res =
            bmalloc(sizeof(struct accum_node) * weight_sz);
        size_t sz = 0;
        while (sz < weight_sz && pq->size > 0) {
            rbc_pq_dequeue(pq, res + sz++);
        }
        for (size_t j = weight_sz, k = 1; j > 0; j--) {
            size_t idx = j - 1;
            if (res[idx].is_set) {
                fprintf(stream, "%d Q0 %s %lu %.4f %s\n", qids.ary[i],
                    res[idx].docno, k++, idx + res[idx].val, id);
            }
        }
        free(res);
        rbc_pq_destroy(pq);
    }
}
