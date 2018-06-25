/*
 * This file is a part of Polyfuse.
 *
 * Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include <CppUTest/TestHarness.h>

extern "C" {
#include "pq.h"

struct pq *pq;
struct dbl_entry stubs[] = {
  {(char *)"DOC-1", true, 1.0, 1},
  {(char *)"DOC-2", true, 2.0, 2},
  {(char *)"DOC-3", true, 3.0, 3},
  {(char *)"DOC-4", true, 4.0, 4},
  {(char *)"DOC-5", true, 5.0, 5},
  {(char *)"DOC-6", true, 6.0, 6},
  {(char *)"DOC-7", true, 7.0, 7},
  {(char *)"DOC-8", true, 8.0, 8},
};

void
helper_fill_pq()
{
  for (size_t i = 0; i < 8; i++) {
    pq_insert(pq, stubs[i].docno, stubs[i].val, stubs[i].count);
  }
}
}

TEST_GROUP(pq)
{
  void setup()
  {
    pq = pq_create(8);
  }

  void teardown()
  {
    pq_destroy(pq);
  }
};

/*
 * Empty queue sanity test
 */
TEST(pq, empty_pq_is_empty)
{
  struct dbl_entry dummy; 

  CHECK_EQUAL(0, pq_size(pq));
  CHECK_EQUAL(0, pq_delete(pq));
  CHECK_EQUAL(0, pq_remove(pq, &dummy));
}

/*
 * Can insert an item into the queue
 */
TEST(pq, can_insert_item)
{
  struct dbl_entry dummy;

  pq_insert(pq, dummy.docno, dummy.val, dummy.count);

  CHECK_EQUAL(1, pq_size(pq));
}

/*
 * Can insert negative item into the queue
 */
TEST(pq, can_insert_neg_item)
{
  struct dbl_entry dummy = {
    (char *)"DOC-1", -7.0, true, 1
  };

  pq_insert(pq, dummy.docno, dummy.val, dummy.count);

  CHECK_EQUAL(1, pq_size(pq));
}

/*
 * Skip insert when full with lower priority
 */
TEST(pq, skip_insert_full_and_low_priority)
{
  struct dbl_entry skipnode = {
    (char *)"DOC-SKIP", -1.0, true, 1
  };

  helper_fill_pq();

  pq_insert(pq, skipnode.docno, skipnode.val, skipnode.count);

  struct dbl_entry res;
  pq_min(pq, &res);
  CHECK_EQUAL(8, pq_size(pq));
  DOUBLES_EQUAL(1.0, res.val, 0.01);
}

/*
 * Can delete an item from the queue
 */
TEST(pq, can_delete_item)
{
  struct dbl_entry dummy;

  pq_insert(pq, dummy.docno, dummy.val, dummy.count);
  pq_delete(pq);

  CHECK_EQUAL(0, pq_size(pq));
}

/*
 * Can fetch the top most item from the queue
 */
TEST(pq, fetch_top_item)
{
  // insert 3.0, 2.0, 1.0
  pq_insert(pq, stubs[2].docno, stubs[2].val, stubs[2].count);
  pq_insert(pq, stubs[1].docno, stubs[1].val, stubs[1].count);
  pq_insert(pq, stubs[0].docno, stubs[0].val, stubs[0].count);
  CHECK_EQUAL(3, pq_size(pq));

  struct dbl_entry res;
  pq_min(pq, &res);
  CHECK_EQUAL(3, pq_size(pq));
  DOUBLES_EQUAL(1.0, res.val, 0.01);
}

/*
 * Can remove items in priority order
 */
TEST(pq, remove_items)
{
  // insert 3.0, 2.0, 1.0
  pq_insert(pq, stubs[2].docno, stubs[2].val, stubs[2].count);
  pq_insert(pq, stubs[1].docno, stubs[1].val, stubs[1].count);
  pq_insert(pq, stubs[0].docno, stubs[0].val, stubs[0].count);
  CHECK_EQUAL(3, pq_size(pq));

  struct dbl_entry res;
  pq_remove(pq, &res);
  CHECK_EQUAL(2, pq_size(pq));
  DOUBLES_EQUAL(1.0, res.val, 0.01);

  pq_remove(pq, &res);
  CHECK_EQUAL(1, pq_size(pq));
  DOUBLES_EQUAL(2.0, res.val, 0.01);

  pq_remove(pq, &res);
  CHECK_EQUAL(0, pq_size(pq));
  DOUBLES_EQUAL(3.0, res.val, 0.01);
}
