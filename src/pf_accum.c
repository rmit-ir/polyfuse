/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "pf_accum.h"

#define LOAD_FACTOR 0.75
#define HASH(s, ht) (str_hash(s) % ht->capacity)
#define NEED_REHASH(ht) ((float)ht->size / ht->capacity > LOAD_FACTOR)

/*
 * Map strings to unsigned integers.
 */
static unsigned long
str_hash(const char *str)
{
    char c;
    unsigned long hash = 2081;

    while ((c = *str++)) {
        hash = hash ^ (c + (hash << 6) + (hash >> 2));
    }

    return hash;
}

/*
 * Get a low prime for linear probing.
 *
 * FIXME: A static table of low primes would be handy here.
 */
static size_t
get_prime(size_t n)
{
    while (n > 2) {
        size_t divisor = 2;
        bool found = true;

        while (divisor * divisor <= n) {
            if (0 == n % divisor) {
                found = false;
                break;
            }
            ++divisor;
        }
        if (found) {
            return n;
        }
        --n;
    }

    return 2;
}

/*
 * Create the hash table.
 */
struct pf_accum *
pf_accum_create(size_t capacity)
{
    struct pf_accum *htable;

    htable = bmalloc(sizeof(struct pf_accum));
    htable->capacity = get_prime(capacity);
    htable->size = 0;
    htable->data = bmalloc(sizeof(struct accum_node) * htable->capacity);

    return htable;
}

/*
 * Free the hash table.
 */
void
pf_accum_free(struct pf_accum *htable)
{
    for (size_t i = 0; i < htable->capacity; i++) {
        if (htable->data[i].is_set) {
            free(htable->data[i].docno);
        }
    }
    free(htable->data);
    free(htable);
}

/*
 * Insert an element into the hash table.
 */
unsigned long
pf_accum_update(struct pf_accum **htable, const char *val, long double score)
{
    unsigned long key;
    unsigned long start_pos;
    struct accum_node *entry;
    struct pf_accum *current;

    current = *htable;
    if (NEED_REHASH(current)) {
        current = pf_accum_rehash(current);
        *htable = current;
    }

    key = HASH(val, current);
    start_pos = key;
    do {
        entry = &current->data[key];
        if (!entry->is_set) {
            entry->docno = strdup(val);
            entry->val = score;
            entry->is_set = true;
            entry->count = 1;
            ++current->size;
            break;
        } else if (0 == strncmp(entry->docno, val, strlen(entry->docno))) {
            entry->val += score;
            entry->count++;
            break;
        }
        ++key;
        key %= current->capacity;
    } while (key != start_pos);

    return key;
}

/*
 * Increase table size and rehash all items.
 */
struct pf_accum *
pf_accum_rehash(struct pf_accum *htable)
{
    struct pf_accum *rehash;
    size_t new_size;

    /* Based from current load factor, take it down to ~25% */
    new_size = htable->size * 4;
    rehash = pf_accum_create(new_size);
    rehash->topic = htable->topic;
    rehash->is_set = htable->is_set;

    for (size_t i = 0; i < htable->capacity; ++i) {
        if (htable->data[i].is_set) {
            pf_accum_update(
                &rehash, htable->data[i].docno, htable->data[i].val);
        }
    }

    pf_accum_free(htable);
    return rehash;
}
