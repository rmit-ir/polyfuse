/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "polyfuse.h"

struct topic_list {
    int *ary;
    size_t size;
};

static enum fusetype fusion = TNONE;
static struct pf_topic *topic_tab = NULL;
static struct topic_list qids = {NULL, 0};

long rrf_k = 0;
double *weights = NULL;
size_t weight_sz = 0;

/*
 * Allocate RBC weight to the longest seen run file.
 */
void
pf_weight_alloc(const double phi, const size_t len)
{
    static bool first = true;
    static double w;
    static double _phi;
    size_t prev = weight_sz;

    // alocate weights for the longest run file
    if (len <= weight_sz) {
        return;
    }

    weight_sz = len;
    if (first) {
        weights = (double *)bmalloc(sizeof(double) * weight_sz);
        w = 1.0 - phi;
        _phi = phi;
        first = false;
    } else {
        weights = (double *)brealloc(weights, sizeof(double) * weight_sz);
    }

    for (size_t i = prev; i < weight_sz; i++) {
        weights[i] = w;
        w *= _phi;
    }
}

void
pf_init(const struct trec_topic *topics)
{
    qids.ary = bmalloc(sizeof(int) * topics->len);
    qids.size = topics->len;
    memcpy(qids.ary, topics->ary, sizeof(int) * topics->len);

    // create an accumulator for each topic
    topic_tab = pf_topic_create(topics->len);
    for (size_t i = 0; i < topics->len; i++) {
        pf_topic_insert(&topic_tab, topics->ary[i]);
    }
}

void
pf_destory()
{
    free(weights);
    free(qids.ary);
    pf_topic_free(topic_tab);
}

void
pf_accumulate(struct trec_run *r)
{
    for (size_t i = 0; i < r->len; i++) {
        size_t rank = r->ary[i].rank - 1;
        if (rank < weight_sz) {
            double score = pf_score(rank + 1, &r->ary[i]);
            struct pf_accum **curr;
            curr = pf_topic_lookup(topic_tab, r->ary[i].qid);
            if (*curr) {
                pf_accum_update(curr, r->ary[i].docno, score);
            }
        }
    }
}

void
pf_set_fusion(const enum fusetype type)
{
    fusion = type;
}

void
pf_set_rrf_k(const long k)
{
    rrf_k = k;
}

double
pf_score(size_t rank, struct trec_entry *tentry)
{
    double s = 0.0;

    switch (fusion) {
    case TCOMBSUM:
    case TCOMBMNZ:
        /*
         * CombMNZ:
         *
         * The count of updates are tracked within `pf_accum_update`. The
         * multiplication for CombMNZ is applied when the entry is added to
         * the priority queue in `pf_present`.
         */
        s = tentry->score;
        break;
    case TRBC:
        s = weights[rank - 1];
        break;
    case TRRF:
        s = 1 / ((double)rrf_k + rank);
        break;
    default:
        break;
    }

    return s;
}

void
pf_present(FILE *stream, const char *id, size_t depth)
{
    double norm = 1.0;

    if (depth < 1) {
        err_exit("`depth` is 0");
    }

    if (depth > weight_sz) {
        depth = weight_sz;
    }

    /* combsum, combmnz normalization */
    if (TCOMBSUM == fusion || TCOMBMNZ == fusion) {
        norm = depth;
    }

    for (size_t i = 0; i < qids.size; i++) {
        struct pf_accum *curr;
        curr = *pf_topic_lookup(topic_tab, qids.ary[i]);
        struct pf_pq *pq = pf_pq_create(weight_sz);
        // this is why we use linear probing
        for (size_t j = 0; j < curr->capacity; j++) {
            double score = 0.0;
            if (!curr->data[j].is_set) {
                continue;
            }
            score = curr->data[j].val;
            /*
             * Apply CombMNZ multiplication
             */
            if (TCOMBMNZ == fusion) {
                score *= curr->data[j].count;
            }
            pf_pq_enqueue(pq, curr->data[j].docno, score);
        }
        struct accum_node *res =
            bmalloc(sizeof(struct accum_node) * weight_sz);
        size_t sz = 0;
        while (sz < weight_sz && pq->size > 0) {
            pf_pq_dequeue(pq, res + sz++);
        }
        for (size_t j = weight_sz, k = 1; j > 0; j--) {
            size_t idx = j - 1;
            if (res[idx].is_set) {
                fprintf(stream, "%d Q0 %s %lu %.8f %s\n", qids.ary[i],
                    res[idx].docno, k++, idx + res[idx].val / norm, id);
            }
        }
        free(res);
        pf_pq_destroy(pq);
    }
}
