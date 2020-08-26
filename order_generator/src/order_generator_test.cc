/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "order_generator.h"

#include <gtest/gtest.h>

TEST(GnenerateRandomNumber, should_generate_random_number_in_range) {
  auto result = GnenerateRandomNumber(5, 10);

  EXPECT_LE(result, 10);
  EXPECT_GE(result, 5);

//  EXPECT_EQ(true, 1);
}

TEST(GnenerateRandomNumber, should_generate_initial_prices) {
  auto result = GnenerateRandomNumber(5, 10);

  EXPECT_LE(result, 10);
  EXPECT_GE(result, 5);

//  EXPECT_EQ(true, 1);
}