/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "pq.h"

#define HEAP_ROOT 1

static int
pq_empty(const struct pq *pq);

static void
pq_sift_up(struct pq *pq, size_t n);

static void
pq_sift_down(struct pq *pq, size_t n);

/*
 * Create a new priority queue
 */
struct pq *
pq_create(size_t size)
{
    struct pq *pq;

    pq = bmalloc(sizeof(*pq));
    pq->heap = bmalloc(sizeof(struct accum_node) * (size + HEAP_ROOT));
    pq->size = 0;
    pq->alloc = size; // ignore the HEAP_ROOT offset

    return pq;
}

/*
 * Destory the priority queue.
 */
void
pq_destroy(struct pq *pq)
{
    if (pq) {
        free(pq->heap);
    }

    free(pq);
}

/*
 * Get the size of the queue
 */
size_t
pq_size(const struct pq *pq)
{
    if (pq) {
        return pq->size;
    }

    return 0;
}

/*
 * Determine if the priority queue is empty.
 */
static int
pq_empty(const struct pq *pq)
{
    return 0 == pq_size(pq);
}

/*
 * Determine if the priority queue is full.
 */
static int
pq_full(const struct pq *pq)
{
    return pq->alloc == pq_size(pq);
}

/*
 * Sift an element up into its correct place.
 */
static void
pq_sift_up(struct pq *pq, size_t n)
{
    struct accum_node *heap = pq->heap;

    while (n > 1 && heap[n / 2].val > heap[n].val) {
        pq_swap(pq, n, n / 2);
        n = n / 2;
    }
}

/*
 * Sift an element down into its correct place.
 */
static void
pq_sift_down(struct pq *pq, size_t n)
{
    struct accum_node *heap = pq->heap;

    while (2 * n <= pq_size(pq)) {
        size_t j = 2 * n;
        if (j < pq_size(pq) && heap[j].val > heap[j + 1].val) {
            j++;
        }

        if (!(heap[n].val > heap[j].val)) {
            break;
        }

        pq_swap(pq, n, j);
        n = j;
    }
}

/*
 * Insert a value with the specified priority.
 */
int
pq_insert(
    struct pq *pq, char *const val, const double prio, const size_t count)
{
    struct accum_node new, top;
    int ret = 0;

    if (!pq || !pq->heap) {
        goto ret;
    }

    // skip if the new node can't make it into the heap, once the heap is full
    if (pq_full(pq) && pq_min(pq, &top) && prio < top.val) {
        goto ret;
    }

    // make room for the new item
    if (pq_full(pq)) {
        pq_delete(pq);
    }

    new.val = prio;
    new.docno = val; // holds a reference to the existing string
    new.is_set = true;
    new.count = count;

    /* heap index begins at 1 */
    pq->heap[++pq->size] = new;
    pq_sift_up(pq, pq_size(pq));
    ret = 1;

ret:
    return ret;
}

/*
 * Peforms a fetch and delete of the top most item.
 */
int
pq_remove(struct pq *pq, struct accum_node *res)
{
    int ret = 0;

    if (!pq || !pq->heap || !res) {
        return ret;
    }

    if (pq_empty(pq)) {
        return ret;
    }

    pq_min(pq, res);
    pq_delete(pq);
    ret = 1;

    return ret;
}

/*
 * Fetch the top item from the priority queue. The item is not removed.
 */
int
pq_min(const struct pq *pq, struct accum_node *res)
{
    int ret = 0;

    if (!pq || !pq->heap || !res) {
        return ret;
    }

    if (pq_empty(pq)) {
        return ret;
    }

    *res = pq->heap[HEAP_ROOT];
    ret = 1;

    return ret;
}

/*
 * Delete the top most item from the priority queue.
 */
int
pq_delete(struct pq *pq)
{
    int ret = 0;

    if (!pq || !pq->heap) {
        return ret;
    }

    if (pq_empty(pq)) {
        return ret;
    }

    /*
     * 1. Move the root to the last spot in the heap
     * 2. Shrink the heap by 1
     * 3. Sift the root node down into it's correct place
     */
    pq->heap[HEAP_ROOT] = pq->heap[pq->size];
    pq->size--;
    pq_sift_down(pq, 1);

    ret = 1;

    return ret;
}

/*
 * Compares two elements with each other.
 */
int
pq_cmp(const struct pq *pq, size_t a, size_t b)
{
    if (!pq || !pq->heap) {
        err_exit("pq_cmp is NULL");
    }

    struct accum_node *heap = pq->heap;

    if (heap[a].val == heap[b].val) {
        return 0;
    } else if (heap[a].val < heap[b].val) {
        return -1;
    } else {
        return 1;
    }
}

/*
 * Swaps two elements with each other.
 */
void
pq_swap(const struct pq *pq, size_t a, size_t b)
{
    if (pq) {
        struct accum_node *heap = pq->heap;
        struct accum_node tmp;
        if (heap) {
            tmp = heap[a];
            heap[a] = heap[b];
            heap[b] = tmp;
        }
    }
}
