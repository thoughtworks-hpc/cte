/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/order_generator.h"

#include <gtest/gtest.h>

TEST(GenerateRandomNumber, should_generate_random_number_in_range) {
  auto result = GenerateRandomNumber(5, 10);

  EXPECT_GE(result, 5);
  EXPECT_LE(result, 10);
}

TEST(GenerateInitialPrice, should_generate_initial_prices_in_range) {
  int symbol_id_min = 1;
  int symbol_id_max = 10;
  int price_min = 100;
  int price_max = 500;
  auto initial_prices =
      GenerateInitialPrice(symbol_id_min, symbol_id_max, price_min, price_max);

  for (const auto& initial_price : initial_prices) {
    EXPECT_GE(initial_price.first, symbol_id_min);
    EXPECT_LE(initial_price.first, symbol_id_max);
    EXPECT_GE(initial_price.second, price_min * 100);
    EXPECT_LE(initial_price.second, price_max * 100);
  }

  EXPECT_EQ(symbol_id_max - symbol_id_min + 1, initial_prices.size());
}

TEST(GenerateOrder, should_generate_order_in_range) {
  int price_min = 100;
  int price_max = 500;
  int order_amount = 10;
  int user_id_min = 1;
  int user_id_max = 100;
  int symbol_id_min = 3;
  int symbol_id_max = 10;
  int amount_min = 100;
  int amount_max = 1000;
  auto initial_prices =
      GenerateInitialPrice(symbol_id_min, symbol_id_max, price_min, price_max);
  for (int i = 0; i < order_amount; i++) {
    Order order(initial_prices, user_id_min, user_id_max, amount_min,
                amount_max);
    int initial_price_now = initial_prices[order.GetSymbol()];
    EXPECT_GE(order.GetUserId(), user_id_min);
    EXPECT_LE(order.GetUserId(), user_id_max);
    EXPECT_GE(order.GetSymbol(), symbol_id_min);
    EXPECT_LE(order.GetSymbol(), symbol_id_max);
    EXPECT_GE(order.GetPrice(), initial_price_now * 0.9);
    EXPECT_LE(order.GetPrice(), initial_price_now * 1.1);
    EXPECT_GE(order.GetAmount(), amount_min);
    EXPECT_LE(order.GetAmount(), amount_max);
  }
}
