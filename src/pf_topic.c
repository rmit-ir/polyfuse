/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "pf_topic.h"

#define LOAD_FACTOR 0.75
#define HASH(num, ht) (int_hash(num) % ht->capacity)
#define NEED_REHASH(ht) ((float)ht->size / ht->capacity > LOAD_FACTOR)

static enum accumtype accum_type = ACCUM_DBL;

void
enable_list_accumulator()
{
    accum_type = ACCUM_LIST;
}

/*
 * Knuth's multiplicative method.
 */
static uint32_t
int_hash(const uint32_t val)
{
    const uint32_t k = 2654435761;

    return val * k;
}

/*
 * Get a low prime for linear probing.
 *
 * FIXME: A static table of low primes would be handy here.
 */
static unsigned long
get_prime(unsigned long n)
{
    while (n > 2) {
        unsigned long divisor = 2;
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
struct pf_topic *
pf_topic_create(size_t capacity)
{
    struct pf_topic *htable;

    htable = bmalloc(sizeof(*htable));
    htable->capacity = get_prime(capacity);
    htable->size = 0;
    htable->data = bmalloc(sizeof(void *) * htable->capacity);

    return htable;
}

/*
 * Free the hash table.
 */
void
pf_topic_free(struct pf_topic *htable)
{
    for (size_t i = 0; i < htable->capacity; i++) {
        if (htable->data[i]) {
            if (ACCUM_LIST == accum_type) {
                accum_list_free((struct accum_list *)htable->data[i]);
            } else {
                accum_dbl_free((struct accum_dbl *)htable->data[i]);
            }
        }
    }
    free(htable->data);
    free(htable);
}

/*
 * Insert a string into the hash table.
 */
unsigned long
pf_topic_insert(struct pf_topic **htable, const int val)
{
    unsigned long key;
    struct accum *entry;
    struct pf_topic *current;

    current = *htable;
    if (NEED_REHASH(current)) {
        current = pf_topic_rehash(current);
        *htable = current;
    }

    key = HASH(val, current);
    /* assume table never gets full */
    entry = current->data[key];
    while (entry && entry->is_set && entry->topic != val) {
        ++key;
        key %= current->capacity;
        entry = current->data[key];
    }

    if (!entry) {
        if (ACCUM_LIST == accum_type) {
            entry = accum_list_create(1000);
        } else {
            entry = accum_dbl_create(1000);
        }
        entry->topic = val;
        entry->is_set = true;
        current->data[key] = entry;
        ++current->size;
    }

    return key;
}

/*
 * Check a value exists in the hash table. Returns the value if found, `NULL`
 * otherwise.
 */
struct accum **
pf_topic_lookup(struct pf_topic *htable, const int val)
{
    unsigned long key;
    struct accum *entry;

    key = HASH(val, htable);
    entry = htable->data[key];
    while (entry && entry->topic != val) {
        ++key;
        key %= htable->capacity;
        entry = htable->data[key];
    }

    return &htable->data[key];
}

/*
 * Increase table size and rehash all items.
 */
struct pf_topic *
pf_topic_rehash(struct pf_topic *htable)
{
    struct pf_topic *rehash;
    size_t new_size;

    /* Based from current load factor, take it down to ~25% */
    new_size = htable->size * 4;
    rehash = pf_topic_create(new_size);

    for (size_t i = 0; i < htable->capacity; ++i) {
        if (htable->data[i]) {
            pf_topic_insert(&rehash, htable->data[i]->topic);
        }
    }

    pf_topic_free(htable);

    return rehash;
}
