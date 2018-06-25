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

static struct accum *
accum_rehash(struct accum *htable);

/*
 * `list_entry` internal array handling
 */
#define LDBL_MIN_CAPACITY 32
#define LDBL_TYPE_SIZE (sizeof(long double))

struct ldbl_arr {
    long double *data;
    size_t size;
    size_t capacity;
};

/*
 * Allocate more memory to the array if required.
 */
static void
ldbl_arr_alloc(struct ldbl_arr *arr)
{
    if (arr) {
        if (0 == arr->capacity) {
            arr->capacity = LDBL_MIN_CAPACITY;
            arr->data = bmalloc(LDBL_TYPE_SIZE * arr->capacity);
        } else if (arr->size == arr->capacity) {
            arr->capacity *= 2;
            arr->data = brealloc(arr->data, LDBL_TYPE_SIZE * arr->capacity);
        }
    }
}

static struct ldbl_arr *
ldbl_arr_create()
{
    struct ldbl_arr *ary;

    ary = (struct ldbl_arr *)bmalloc(sizeof(struct ldbl_arr));
    ldbl_arr_alloc(ary);

    return ary;
}

static void
ldbl_arr_destroy(struct ldbl_arr *arr)
{
    arr->capacity = 0;
    arr->size = 0;
    free(arr->data);
}

static void
ldbl_arr_insert(struct ldbl_arr *arr, const long double val)
{
    if (arr) {
        if (0 == arr->size) {
            arr->data[arr->size++] = val;
            return;
        }

        ldbl_arr_alloc(arr);

        // if `val` is largest, insert at end, i.e. `arr->size`.
        size_t idx = arr->size, n = 0;
        for (size_t i = arr->size - 1; (int)i >= 0; i--) {
            if (val < arr->data[i]) {
                idx = i;
                break;
            }
        }
        n = (arr->size - idx) * LDBL_TYPE_SIZE;
        if (n > 0) {
            memmove(arr->data + idx + 1, arr->data + idx, n);
        }
        arr->data[idx] = val;
        ++arr->size;
    }
}
/* end `list_entry` internal array handling */

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
 * Create `long double` accumulator.
 */
struct accum *
accum_dbl_create(const size_t capacity)
{
    struct accum_dbl *dbltab;

    dbltab = bmalloc(sizeof(*dbltab));
    dbltab->type = ACCUM_DBL;
    dbltab->capacity = get_prime(capacity);
    dbltab->size = 0;
    dbltab->is_set = false;
    dbltab->data = bmalloc(sizeof(struct dbl_entry) * dbltab->capacity);

    return (struct accum *)dbltab;
}

/*
 * Free `long double` accumulator.
 */
void
accum_dbl_free(struct accum_dbl *acc)
{
    struct accum_dbl *dbltab = (struct accum_dbl *)acc;
    for (size_t i = 0; i < dbltab->capacity; i++) {
        if (dbltab->data[i].is_set) {
            free(dbltab->data[i].docno);
        }
    }
    free(dbltab->data);
    free(dbltab);
}

/*
 * `accum_dbl` accumulator operations (internal).
 */
enum accum_op { OP_NONE, OP_ADD, OP_LESS, OP_GREATER };

/*
 * Update an element in the hash table.
 */
static unsigned long
accum_dbl_modify_(struct accum **htable, const char *docno, long double score,
    const enum accum_op op)
{
    unsigned long key;
    unsigned long start_pos;
    struct accum_dbl *current;
    struct dbl_entry *entry;

    if (NEED_REHASH((*htable))) {
        *htable = accum_rehash(*htable);
    }
    current = (struct accum_dbl *)(*htable);

    key = HASH(docno, current);
    start_pos = key;
    do {
        entry = &current->data[key];
        if (!entry->is_set) {
            entry->docno = strdup(docno);
            entry->val = score;
            entry->is_set = true;
            entry->count = 1;
            ++current->size;
            break;
        } else if (0 == strncmp(entry->docno, docno, strlen(entry->docno))) {
            switch (op) {
            case OP_LESS:
                if (score < entry->val) {
                    entry->val = score;
                }
                break;
            case OP_GREATER:
                if (score > entry->val) {
                    entry->val = score;
                }
                break;
            case OP_ADD:
            default:
                entry->val += score;
                break;
            }
            entry->count++;
            break;
        }
        ++key;
        key %= current->capacity;
    } while (key != start_pos);

    return key;
}

/*
 * Set accumulator only if `score` is less than the current value.
 */
unsigned long
accum_dbl_less(struct accum **htable, const char *docno, long double score)
{
    /* struct accum_dbl **dbltab = (struct accum_dbl **)htable; */
    return accum_dbl_modify_(htable, docno, score, OP_LESS);
}

/*
 * Set accumulator only if `score` is greater than the current value.
 */
unsigned long
accum_dbl_greater(struct accum **htable, const char *docno, long double score)
{
    /* struct accum_dbl **dbltab = (struct accum_dbl **)htable; */
    return accum_dbl_modify_(htable, docno, score, OP_GREATER);
}

/*
 * Accumulate value.
 */
unsigned long
accum_dbl_update(struct accum **htable, const char *docno, long double score)
{
    /* struct accum_dbl **dbltab = (struct accum_dbl **)htable; */
    return accum_dbl_modify_(htable, docno, score, OP_ADD);
}

/*
 * Create `list` accumulator.
 */
struct accum *
accum_list_create(const size_t capacity)
{
    struct accum_list *tab;

    tab = bmalloc(sizeof(*tab));
    tab->type = ACCUM_LIST;
    tab->capacity = get_prime(capacity);
    tab->size = 0;
    tab->data = bmalloc(sizeof(struct list_entry) * tab->capacity);

    return (struct accum *)tab;
}

/*
 * Free `list` accumulator.
 */
void
accum_list_free(struct accum_list *acc)
{
    struct accum_list *tab = (struct accum_list *)acc;
    for (size_t i = 0; i < tab->capacity; i++) {
        if (tab->data[i].is_set) {
            free(tab->data[i].docno);
            ldbl_arr_destroy(tab->data[i].ary);
            free(tab->data[i].ary);
        }
    }
    free(tab->data);
    free(tab);
}

/*
 * Find the median value.
 */
long double
accum_list_median(const struct list_entry *l)
{
    long double m = 0.0;
    size_t idx;

    if (l->ary->size % 2 == 0) {
        idx = l->ary->size >> 1;
        m = l->ary->data[idx];
        m += l->ary->data[idx + 1];
        m /= 2;
    } else {
        idx = l->ary->size >> 1;
        m = l->ary->data[idx + 1];
    }

    return m;
}

#define ACCUM_LIST_SZ (sizeof(struct list_entry))

/*
 * Append an item to the list accumulator.
 */
unsigned long
accum_list_append(struct accum **htable, const char *docno, long double score)
{
    unsigned long key;
    unsigned long start_pos;
    struct list_entry *entry;
    struct accum_list *current;
    uint8_t *dat;

    if (NEED_REHASH((*htable))) {
        *htable = accum_rehash(*htable);
    }
    current = (struct accum_list *)(*htable);

    key = HASH(docno, current);
    start_pos = key;
    dat = (uint8_t *)current->data + key * ACCUM_LIST_SZ;
    do {
        entry = (struct list_entry *)dat;
        if (!entry->is_set) {
            entry->docno = strdup(docno);
            entry->ary = ldbl_arr_create();
            ldbl_arr_insert(entry->ary, score);
            entry->is_set = true;
            ++current->size;
            break;
        } else if (0 == strncmp(entry->docno, docno, strlen(entry->docno))) {
            ldbl_arr_insert(entry->ary, score);
            break;
        }
        ++key;
        key %= current->capacity;
    } while (key != start_pos);

    return key;
}

static void
accum_dbl_rehash(struct accum_dbl *old, struct accum *new)
{
    for (size_t i = 0; i < old->capacity; ++i) {
        if (old->data[i].is_set) {
            accum_dbl_update(&new, old->data[i].docno, old->data[i].val);
        }
    }
    accum_dbl_free(old);
}

static void
accum_list_rehash(struct accum_list *old, struct accum *new)
{
    for (size_t i = 0; i < old->capacity; ++i) {
        if (old->data[i].is_set) {
            for (size_t j = 0; j < old->data[i].ary->size; ++j) {
                long double val = old->data[i].ary->data[j];
                accum_list_append(&new, old->data[i].docno, val);
            }
        }
    }
    accum_list_free(old);
}

/*
 * Increase table size and rehash all items.
 */
static struct accum *
accum_rehash(struct accum *htable)
{
    struct accum *rehash;
    size_t new_size;

    /* Based from current load factor, take it down to ~25% */
    new_size = htable->size * 4;
    if (ACCUM_LIST == htable->type) {
        rehash = accum_list_create(new_size);
    } else {
        rehash = accum_dbl_create(new_size);
    }

    rehash->topic = htable->topic;
    rehash->is_set = htable->is_set;

    if (ACCUM_LIST == htable->type) {
        accum_list_rehash((struct accum_list *)htable, rehash);
    } else {
        accum_dbl_rehash((struct accum_dbl *)htable, rehash);
    }

    return rehash;
}
