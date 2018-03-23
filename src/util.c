/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "util.h"

/* Exit with failure if calloc fails */
void *
bmalloc(size_t size)
{
    void *ptr;

    ptr = calloc(1, size);
    if (!ptr) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

/*
 * Exit with failure if realloc fails.
 */
void *
brealloc(void *old_mem, size_t new_size)
{
    old_mem = realloc(old_mem, new_size);
    if (!old_mem) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }

    return old_mem;
}

/*
 * Exit with error message.
 */
void
err_exit(const char *s, ...)
{
    va_list ap;

    va_start(ap, s);
    fprintf(stderr, "error: ");
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(EXIT_FAILURE);
}
