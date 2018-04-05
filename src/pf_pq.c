/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "pf_pq.h"

#define HEAP_ROOT 1
#define MAX_HEAP_SIZE (256 * 256 * 16 - 1)
#define HEAP_ARRAY_SIZE (MAX_HEAP_SIZE + 1)

static int
pq_empty(const struct pf_pq *pq);

static void
pq_heap_sift_top(struct accum_node *heap, int size);

static void
pq_heap_sift_bottom(struct accum_node *heap, int size);

static void
pq_heap_swap(struct accum_node *heap, int a, int b);

static int
pq_heap_cmp(struct accum_node *heap, int a, int b);

/*
 * Create a new priority queue
 */
struct pf_pq *
pf_pq_create(size_t size)
{
    struct pf_pq *pq;

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
pf_pq_destroy(struct pf_pq *pq)
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
pf_pq_size(const struct pf_pq *pq)
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
pq_empty(const struct pf_pq *pq)
{
    return 0 == pf_pq_size(pq);
}

/*
 * Determine if the priority queue is full.
 */
static int
pq_full(const struct pf_pq *pq)
{
    return pq->alloc == pf_pq_size(pq);
}

/*
 * Sift the root element down into the min heap's correct place.
 */
static void
pq_heap_sift_top(struct accum_node *heap, const int size)
{
    int current, height;

    if (!heap || size < 2) {
        return;
    }

    current = HEAP_ROOT;
    height = floor(log2(size));

    do {
        int min, left, right;

        left = current * 2;
        right = current * 2 + 1;
        min = right;

        /* The current node has no children. */
        if (left > size) {
            break;
        }

        /*
         * Check the left child if the current node does not have a right
         * child or if the left child is smaller than the right child.
         */
        if (right > size) {
            min = left;
        } else if (heap[left].val <= heap[right].val) {
            min = left;
        }

        if ((pq_heap_cmp(heap, current, min)) < 0) {
            break;
        }

        pq_heap_swap(heap, current, min);
        current = min;
    } while (--height);
}

/*
 * Sift the last inserted element into the min heap's correct place.
 */
static void
pq_heap_sift_bottom(struct accum_node *heap, const int size)
{
    int current, parent;

    if (!heap || size < 2) {
        return;
    }

    current = size;
    parent = size / 2;

    while (parent > 0 && heap[current].val < heap[parent].val) {
        pq_heap_swap(heap, current, parent);

        current = parent;
        parent /= 2;
    }
}

/*
 * Insert a value with the specified priority.
 */
int
pf_pq_enqueue(struct pf_pq *pq, char *const val, const double prio)
{
    struct accum_node new, top;
    int ret = 0;

    if (!pq || !pq->heap) {
        goto ret;
    }

    // skip if the new node can't make it into the heap, once the heap is full
    if (pq_full(pq) && pf_pq_find(pq, &top) && prio < top.val) {
        goto ret;
    }

    // make room for the new item
    if (pq_full(pq)) {
        pf_pq_delete(pq);
    }

    // there can be no negative priority since we are summing the RBP weight
    if (prio < 0) {
        goto ret;
    }

    new.val = prio;
    new.docno = val; // holds a reference to the existing string
    new.is_set = true;

    /* heap index begins at 1 */
    pq->heap[++pq->size] = new;
    pq_heap_sift_bottom(pq->heap, pq->size);
    ret = 1;

ret:
    return ret;
}

/*
 * Fetch the top item from the priority queue. The item is not removed.
 */
int
pf_pq_find(const struct pf_pq *pq, struct accum_node *acc_node)
{
    int ret = 0;

    if (!pq || !pq->heap || !acc_node) {
        return ret;
    }

    if (pq_empty(pq)) {
        return ret;
    }
    *acc_node = pq->heap[HEAP_ROOT];
    ret = 1;

    return ret;
}

/*
 * Delete the top most item from the priority queue.
 */
int
pf_pq_delete(struct pf_pq *pq)
{
    int ret = 0;

    if (!pq || !pq->heap) {
        return ret;
    }

    if (pq_empty(pq)) {
        return ret;
    }

    /*
     * 1. Swap the root with the last node in the heap
     * 2. Shrink the heap by 1
     * 3. Sift the root node down into it's correct place
     *
     * No need for the swap, since we're deleting it anyway. Just move the
     * last node to the root of the heap.
     */
    pq->heap[HEAP_ROOT] = pq->heap[pq->size];
    pq->size--;
    pq_heap_sift_top(pq->heap, pq->size);

    ret = 1;

    return ret;
}

/*
 * Peforms a fetch and delete of the top most item.
 */
int
pf_pq_dequeue(struct pf_pq *pq, struct accum_node *acc_node)
{
    int ret = 0;

    if (!pq || !pq->heap || !acc_node) {
        return ret;
    }

    if (pq_empty(pq)) {
        return ret;
    }

    pf_pq_find(pq, acc_node);
    pf_pq_delete(pq);
    ret = 1;

    return ret;
}

/*
 * Compares two pq elements with each other.
 */
int
pf_pq_cmp(const struct pf_pq *pq, int a, int b)
{
    if (!pq || !pq->heap) {
        err_exit("pq_cmp is NULL");
    }

    return pq_heap_cmp(pq->heap, a, b);
}

/*
 * Swaps two pq elements with each other.
 */
void
pf_pq_swap(const struct pf_pq *pq, int a, int b)
{
    if (pq) {
        pq_heap_swap(pq->heap, a, b);
    }
}

/*
 * Swap two elements with each other.
 */
static void
pq_heap_swap(struct accum_node *heap, int a, int b)
{
    struct accum_node tmp;

    if (heap) {
        tmp = heap[a];
        heap[a] = heap[b];
        heap[b] = tmp;
    }
}

/*
 * Compare two heap elements with each other.
 */
static int
pq_heap_cmp(struct accum_node *heap, int a, int b)
{
    if (heap[a].val == heap[b].val) {
        return 0;
    } else if (heap[a].val < heap[b].val) {
        return -1;
    } else {
        return 1;
    }
}
