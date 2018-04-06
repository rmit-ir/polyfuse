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
#include "pf_pq.h"

struct pf_pq *pq;
struct accum_node stubs[] = {
  {(char *)"DOC-1", 1.0, true, 1},
  {(char *)"DOC-2", 2.0, true, 1},
  {(char *)"DOC-3", 3.0, true, 1},
  {(char *)"DOC-4", 4.0, true, 1},
  {(char *)"DOC-5", 5.0, true, 1},
  {(char *)"DOC-6", 6.0, true, 1},
  {(char *)"DOC-7", 7.0, true, 1},
  {(char *)"DOC-8", 8.0, true, 1},
};

void
helper_fill_pq()
{
  for (size_t i = 0; i < 8; i++) {
    pf_pq_enqueue(pq, stubs[i].docno, stubs[i].val);
  }
}
}

TEST_GROUP(pf_pq)
{
  void setup()
  {
    pq = pf_pq_create(8);
  }

  void teardown()
  {
    pf_pq_destroy(pq);
  }
};

/*
 * Empty queue sanity test
 */
TEST(pf_pq, empty_pq_is_empty)
{
  struct accum_node dummy; 

  CHECK_EQUAL(0, pf_pq_size(pq));
  CHECK_EQUAL(0, pf_pq_delete(pq));
  CHECK_EQUAL(0, pf_pq_dequeue(pq, &dummy));
}

/*
 * Can insert an item into the queue
 */
TEST(pf_pq, can_insert_item)
{
  struct accum_node dummy;

  pf_pq_enqueue(pq, dummy.docno, dummy.val);

  CHECK_EQUAL(1, pf_pq_size(pq));
}

/*
 * Can insert negative item into the queue
 */
TEST(pf_pq, can_insert_neg_item)
{
  struct accum_node dummy = {
    (char *)"DOC-1", -7.0, true, 1
  };

  pf_pq_enqueue(pq, dummy.docno, dummy.val);

  CHECK_EQUAL(1, pf_pq_size(pq));
}

/*
 * Skip insert when full with lower priority
 */
TEST(pf_pq, skip_insert_full_and_low_priority)
{
  struct accum_node skipnode = {
    (char *)"DOC-SKIP", -1.0, true, 1
  };

  helper_fill_pq();

  pf_pq_enqueue(pq, skipnode.docno, skipnode.val);

  struct accum_node res;
  pf_pq_find(pq, &res);
  CHECK_EQUAL(8, pf_pq_size(pq));
  DOUBLES_EQUAL(1.0, res.val, 0.01);
}

/*
 * Can delete an item from the queue
 */
TEST(pf_pq, can_delete_item)
{
  struct accum_node dummy;

  pf_pq_enqueue(pq, dummy.docno, dummy.val);
  pf_pq_delete(pq);

  CHECK_EQUAL(0, pf_pq_size(pq));
}

/*
 * Can fetch the top most item from the queue
 */
TEST(pf_pq, fetch_top_item)
{
  // insert 3.0, 2.0, 1.0
  pf_pq_enqueue(pq, stubs[2].docno, stubs[2].val);
  pf_pq_enqueue(pq, stubs[1].docno, stubs[1].val);
  pf_pq_enqueue(pq, stubs[0].docno, stubs[0].val);
  CHECK_EQUAL(3, pf_pq_size(pq));

  struct accum_node res;
  pf_pq_find(pq, &res);
  CHECK_EQUAL(3, pf_pq_size(pq));
  DOUBLES_EQUAL(1.0, res.val, 0.01);
}

/*
 * Can dequeue items in priority order
 */
TEST(pf_pq, dequeue_items)
{
  // insert 3.0, 2.0, 1.0
  pf_pq_enqueue(pq, stubs[2].docno, stubs[2].val);
  pf_pq_enqueue(pq, stubs[1].docno, stubs[1].val);
  pf_pq_enqueue(pq, stubs[0].docno, stubs[0].val);
  CHECK_EQUAL(3, pf_pq_size(pq));

  struct accum_node res;
  pf_pq_dequeue(pq, &res);
  CHECK_EQUAL(2, pf_pq_size(pq));
  DOUBLES_EQUAL(1.0, res.val, 0.01);

  pf_pq_dequeue(pq, &res);
  CHECK_EQUAL(1, pf_pq_size(pq));
  DOUBLES_EQUAL(2.0, res.val, 0.01);

  pf_pq_dequeue(pq, &res);
  CHECK_EQUAL(0, pf_pq_size(pq));
  DOUBLES_EQUAL(3.0, res.val, 0.01);
}
