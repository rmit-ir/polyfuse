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
long double *weights = NULL;
size_t weight_sz = 0;

/*
 * Allocate RBC weight to the deepest topic seen in all run files.
 */
void
pf_weight_alloc(const long double phi, const size_t depth)
{
    static bool first = true;
    static long double w;
    static long double _phi;
    size_t prev = weight_sz;

    if (depth <= weight_sz) {
        return;
    }

    weight_sz = depth;
    if (first) {
        weights = (long double *)bmalloc(sizeof(long double) * weight_sz);
        w = 1.0 - phi;
        _phi = phi;
        first = false;
    } else {
        weights =
            (long double *)brealloc(weights, sizeof(long double) * weight_sz);
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
            long double score = pf_score(rank + 1, r->len, &r->ary[i]);
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

long double
pf_score(size_t rank, size_t n, struct trec_entry *tentry)
{
    long double s = 0.0;

    switch (fusion) {
    case TBORDA:
        s = ((long double)n - rank + 1) / n;
        break;
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
    case TISR:
    case TLOGISR:
        /*
         * ISR and logISR:
         *
         * The count of updates are tracked within `pf_accum_update`. The
         * multiplication for ISR, logISR is applied when the entry is added to
         * the priority queue in `pf_present`.
         */
        s = (long double)1 / pow(rank, 2);
        break;
    case TRBC:
        s = weights[rank - 1];
        break;
    case TRRF:
        s = 1 / ((long double)rrf_k + rank);
        break;
    default:
        break;
    }

    return s;
}

void
pf_present(FILE *stream, const char *id, size_t depth)
{
    if (depth < 1) {
        err_exit("`depth` is 0");
    }

    if (depth > weight_sz) {
        depth = weight_sz;
    }

    for (size_t i = 0; i < qids.size; i++) {
        struct pf_accum *curr;
        curr = *pf_topic_lookup(topic_tab, qids.ary[i]);
        struct pq *pq = pq_create(weight_sz);
        // this is why we use linear probing
        for (size_t j = 0; j < curr->capacity; j++) {
            long double score = 0.0;
            if (!curr->data[j].is_set) {
                continue;
            }
            score = curr->data[j].val;
            /*
             * Apply multiplication for CombMNZ, ISR and logISR.
             */
            if (TCOMBMNZ == fusion || TISR == fusion) {
                score *= curr->data[j].count;
            } else if (TLOGISR == fusion) {
                /* +1 to `log` to avoid log(1) = 0 */
                score *= log(curr->data[j].count + 1);
            }
            pq_insert(pq, curr->data[j].docno, score, curr->data[j].count);
        }
        struct accum_node *res =
            bmalloc(sizeof(struct accum_node) * weight_sz);
        size_t sz = 0;
        while (sz < weight_sz && pq->size > 0) {
            pq_remove(pq, res + sz++);
        }
        for (size_t j = sz, k = 1; (int)depth >= 0; j--, depth--) {
            if (res[j].is_set) {
                fprintf(stream, "%d Q0 %s %lu %.9Lf %s\n", qids.ary[i],
                    res[j].docno, k++, j + res[j].val, id);
            }
            if (0 == j) {
                break;
            }
        }
        free(res);
        pq_destroy(pq);
    }
}
