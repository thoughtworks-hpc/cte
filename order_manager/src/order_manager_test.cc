/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_manager.h"

#include <gtest/gtest.h>

#include "../include/order_store_mock.h"

TEST(OrderManager, should_store_order_correctly_when_place_an_order) {
  auto order_store = std::make_shared<OrderStoreMock>();
  std::vector<std::shared_ptr<Channel>> empty_channels;
  OrderManagerService order_manager(order_store, nullptr, empty_channels);

  ::order_manager_proto::Order order;
  ::order_manager_proto::Reply reply;
  order.set_amount(7);
  order.set_user_id(6);
  order.set_price(27);
  order.set_symbol(1);
  order.set_trading_side(order_manager_proto::TRADING_BUY);

  order_manager.PlaceOrder(nullptr, &order, &reply);

  match_engine_proto::Order order_stored;
  std::string status_stored;
  order_store->GetOrderByIndex(0, order_stored, status_stored);

  EXPECT_EQ(order_stored.order_id(), 1);
  EXPECT_EQ(order_stored.amount(), 7);
  EXPECT_EQ(order_stored.user_id(), 6);
  EXPECT_EQ(order_stored.price(), 27);
  EXPECT_EQ(order_stored.symbol(), 1);
  EXPECT_EQ(order_stored.trading_side(), order_manager_proto::TRADING_BUY);
  auto submit_time = order_stored.submit_time();
  EXPECT_GT(submit_time.seconds(), 0);
  EXPECT_GT(submit_time.nanos(), 0);
}

TEST(OrderManager, should_auto_increment_order_id_when_place_orders) {
  auto order_store = std::make_shared<OrderStoreMock>();
  std::vector<std::shared_ptr<Channel>> empty_channels;
  OrderManagerService order_manager(order_store, nullptr, empty_channels);

  ::order_manager_proto::Order order1;
  ::order_manager_proto::Order order2;
  ::order_manager_proto::Order order3;
  ::order_manager_proto::Reply reply;

  order_manager.PlaceOrder(nullptr, &order1, &reply);
  order_manager.PlaceOrder(nullptr, &order2, &reply);
  order_manager.PlaceOrder(nullptr, &order3, &reply);

  match_engine_proto::Order order_stored1;
  match_engine_proto::Order order_stored2;
  match_engine_proto::Order order_stored3;
  std::string status_stored;
  order_store->GetOrderByIndex(0, order_stored1, status_stored);
  order_store->GetOrderByIndex(1, order_stored2, status_stored);
  order_store->GetOrderByIndex(2, order_stored3, status_stored);

  EXPECT_EQ(order_stored1.order_id(), 1);
  EXPECT_EQ(order_stored2.order_id(), 2);
  EXPECT_EQ(order_stored3.order_id(), 3);
}
