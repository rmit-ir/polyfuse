/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "trec.h"

#define INIT_SZ 16

const char *
trec_norm_str[] = {
  "none",
  "min-max",
  "sum",
  "standard (zmuv)"
};

static int prev_top = 0;

/*
 * Allocate more memory if required.
 */
static void
trec_entry_alloc(struct trec_run *r)
{
    if (r && r->len == r->alloc) {
        r->alloc *= 2;
        r->ary = brealloc(r->ary, sizeof(struct trec_entry) * r->alloc);
    }
}

/*
 * Allocate more memory if required.
 */
static void
trec_topic_alloc(struct trec_topic *t)
{
    if (t && t->len == t->alloc) {
        t->alloc *= 2;
        t->ary = brealloc(t->ary, sizeof(int) * t->alloc);
    }
}

/**
 * Alloc a new trec run array.
 */
struct trec_run *
trec_create()
{
    struct trec_run *run = NULL;

    run = bmalloc(sizeof(struct trec_run));
    run->ary = bmalloc(sizeof(struct trec_entry) * INIT_SZ);
    run->len = 0;
    run->alloc = INIT_SZ;

    run->topics.ary = bmalloc(sizeof(int) * INIT_SZ);
    run->topics.len = 0;
    run->topics.alloc = INIT_SZ;

    return run;
}

void
trec_destroy(struct trec_run *run)
{
    if (run) {
        for (size_t i = 0; i < run->len; i++) {
            free(run->ary[i].docno);
            free(run->ary[i].name);
        }
        free(run->ary);
        free(run->topics.ary);
        free(run);
    }
}

static struct trec_entry
parse_line(char *line, int *topic)
{
    static int rank = 1;
    const char *delim = "\t ";
    const int num_sep = 5; // 6 columns
    struct trec_entry tentry;
    char *dup = strndup(line, strlen(line));
    char *tok, *p;
    int c = 0;
    int ch;

    p = line;
    while ((ch = *p++)) {
        if (isspace(ch)) {
            c++;
        }
    }
    if (c != num_sep) {
        err_exit("found %d fields but should be %d", c, num_sep + 1);
    }

    tok = strtok(dup, delim);
    tentry.qid = strtol(tok, NULL, 10);

    if (prev_top != tentry.qid) {
        rank = 1;
        prev_top = tentry.qid;
        *topic = tentry.qid;
    }

    tok = strtok(NULL, delim); // skip over column 2
    tok = strtok(NULL, delim);
    tentry.docno = strndup(tok, strlen(tok));
    tok = strtok(NULL, delim); // skip rank column
    tentry.rank = rank++;
    strtol(tok, NULL, 10);
    tok = strtok(NULL, delim);
    tentry.score = strtod(tok, NULL);
    tok = strtok(NULL, delim);
    tentry.name = strndup(tok, strlen(tok));

    free(dup);

    return tentry;
}

void
trec_read(struct trec_run *r, FILE *fp)
{
    char buf[BUFSIZ] = {0};
    int curr_topic;

    prev_top = 0;

    while (fgets(buf, BUFSIZ, fp)) {
        if (buf[strlen(buf) - 1] != '\n') {
            err_exit("input line exceeds %d", BUFSIZ);
        }
        buf[strlen(buf) - 1] = '\0';
        curr_topic = 0;

        trec_entry_alloc(r);
        r->ary[r->len++] = parse_line(buf, &curr_topic);

        trec_topic_alloc(&r->topics);
        if (curr_topic > 0) {
            r->topics.ary[r->topics.len++] = curr_topic;
        }
    }
}

static void
minmax_normalizer(struct trec_run *r)
{
    long double min = 0.0, max = 0.0;
    bool first = true;

    for (size_t i = 0; i < r->len; i++) {
        if (first) {
            min = max = r->ary[i].score;
            first = false;
        }

        if (r->ary[i].score < min) {
            min = r->ary[i].score;
        }
        if (r->ary[i].score > max) {
            max = r->ary[i].score;
        }
    }

    if ((max - min) == 0) {
        DLOG("min - max is zero.");
        return;
    }

    for (size_t i = 0; i < r->len; i++) {
        r->ary[i].score = (r->ary[i].score - min) / (max - min);
    }
}

static void
sum_normalizer(struct trec_run *r)
{
    long double total = 0.0;

    for (size_t i = 0; i < r->len; i++) {
        r->ary[i].score = fabsl(r->ary[i].score);
        total += r->ary[i].score;
    }

    for (size_t i = 0; i < r->len; i++) {
        r->ary[i].score /= total;
    }
}

static void
zmuv_normalizer(struct trec_run *r)
{
    long double mean = 0.0, var = 0.0, std = 0.0;

    for (size_t i = 0; i < r->len; i++) {
        mean += r->ary[i].score;
    }
    mean /= r->len;

    for (size_t i = 0; i < r->len; i++) {
        long double x = r->ary[i].score - mean;
        var += x * x;
    }
    var /= r->len;
    std = sqrtl(var);

    if (std == 0) {
        DLOG("std is zero.");
        return;
    }

    for (size_t i = 0; i < r->len; i++) {
        r->ary[i].score = (r->ary[i].score - mean) / std;
    }
}

void
trec_normalize(struct trec_run *r, enum trec_norm norm)
{
    switch (norm) {
    case TNORM_MINMAX:
        minmax_normalizer(r);
        break;
    case TNORM_SUM:
        sum_normalizer(r);
        break;
    case TNORM_ZMUV:
        zmuv_normalizer(r);
        break;
    case TNORM_NONE:
    default:
        break;
    }
}
