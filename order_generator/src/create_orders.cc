/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include "order_generator.h"

int main() {
  srand((unsigned)time(nullptr));
  std::cout << GnenerateRandomNumber(1,20) <<std::endl;
  auto initial_prices = GetAllInitialPrice();
  // 添加order的数量做为变量
//  GenerateInitialPrice(0,10,1,20);
//
//  auto all_initial_prices = GetAllInitialPrice();
//
//  for (int i = 0; i < 20; i++) {
//    Order order(all_initial_prices, 1, 10, 1, 10);
//    order.CreateOrderInDatabase();
//  }

  return 0;
}