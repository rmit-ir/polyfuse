/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef DEBUG
#define DLOG(...)
#else
#define DLOG(...)                     \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)
#endif /* DEBUG */

void *
bmalloc(size_t size);

void *
brealloc(void *old_mem, size_t new_size);

void
err_exit(const char *s, ...);

#endif /* UTIL_H */
