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
#include "rbc.h"

extern double *weights;
extern size_t weight_sz;
}

TEST_GROUP(rbc)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(rbc, rbc_weight_zero)
{
  const double result[] = {
    0.2, 0.16, 0.128, 0.1024, 0.08192,
    0.065536, 0.0524288, 0.04194304, 0.03355443, 0.02684355,
  };

  /* initial state */
  POINTERS_EQUAL(NULL, weights);
  CHECK_EQUAL(0, weight_sz);

  /* try to allocate zero */
  rbc_weight_alloc(0.8, 0);

  POINTERS_EQUAL(NULL, weights);
  CHECK_EQUAL(0, weight_sz);

  /* compute first 5 */
  rbc_weight_alloc(0.8, 5);

  CHECK_EQUAL(5, weight_sz);
  DOUBLES_EQUAL(result[0], weights[0], 0.01);
  DOUBLES_EQUAL(result[1], weights[1], 0.001);
  DOUBLES_EQUAL(result[2], weights[2], 0.0001);
  DOUBLES_EQUAL(result[3], weights[3], 0.00001);
  DOUBLES_EQUAL(result[4], weights[4], 0.000001);

  /* compute next 5 */
  rbc_weight_alloc(0.8, 10);

  CHECK_EQUAL(10, weight_sz);
  DOUBLES_EQUAL(result[5], weights[5], 0.0000001);
  DOUBLES_EQUAL(result[6], weights[6], 0.00000001);
  DOUBLES_EQUAL(result[7], weights[7], 0.00000001);
  DOUBLES_EQUAL(result[8], weights[8], 0.00000001);
  DOUBLES_EQUAL(result[9], weights[9], 0.00000001);

  /* try to allocate less than the last allocation 5 */
  rbc_weight_alloc(0.8, 3);

  CHECK_FALSE(!weights);
  CHECK_EQUAL(10, weight_sz);
}
