/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#define CAF_SUITE yanghui

#include "../include/match_actor.h"

#include <cdcf/actor_system.h>

#include <caf/test/unit_test_impl.hpp>

#include "../include/match_result_sender_actor.h"

namespace {
struct fixture {
  caf::actor_system_config cfg;
  caf::actor_system sys;
  caf::scoped_actor self;
  fixture() : sys(cfg), self(sys) {
    // nop
  }
};
}  // namespace

class CheckResult : public match_engine::SenderMatchInterface {
 public:
  void SendMatchResult(const match_engine::TradeList& trade_list) override {
    result_data_.insert(result_data_.end(), trade_list.begin(),
                        trade_list.end());
  }

  match_engine::TradeList result_data_;
};

CAF_TEST_FIXTURE_SCOPE(yanghui_tests, fixture)

CAF_TEST(match_only_one_order) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  match_engine::RawOrder order{};

  order.order_id = 1;
  order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  order.user_id = 10;
  order.amount = 100;
  order.price = 1000;
  order.symbol_id = 66;
  order.trading_side = match_engine::TRADING_SITE_BUY;

  self->request(match_actor, caf::infinite, order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });
}

CAF_TEST(match_two_same_amount_order) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  CheckResult check_result;
  auto result_actor =
      sys.spawn(match_engine::MatchResultDealActor, &check_result);

  match_engine::RawOrder buy_order{};

  buy_order.order_id = 1;
  buy_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order.user_id = 10;
  buy_order.amount = 100;
  buy_order.price = 1000;
  buy_order.symbol_id = 66;
  buy_order.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder sell_order{};

  sell_order.order_id = 2;
  sell_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order.user_id = 20;
  sell_order.amount = 100;
  sell_order.price = 900;
  sell_order.symbol_id = 66;
  sell_order.trading_side = match_engine::TRADING_SITE_SELL;

  self->request(match_actor, caf::infinite, result_actor)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  std::this_thread::sleep_for(std::chrono::seconds(1));

  CAF_CHECK(1 == check_result.result_data_.size());
  CAF_CHECK(1000 == check_result.result_data_[0].price);
  CAF_CHECK(1 == check_result.result_data_[0].maker_id);
  CAF_CHECK(2 == check_result.result_data_[0].taker_id);
  CAF_CHECK(100 == check_result.result_data_[0].amount);
  CAF_CHECK(20 == check_result.result_data_[0].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[0].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[0].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_SELL ==
            check_result.result_data_[0].trading_side);
}

CAF_TEST(match_two_diff_amount_order) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  CheckResult check_result;
  auto result_actor =
      sys.spawn(match_engine::MatchResultDealActor, &check_result);

  match_engine::RawOrder buy_order{};

  buy_order.order_id = 1;
  buy_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order.user_id = 10;
  buy_order.amount = 100;
  buy_order.price = 1000;
  buy_order.symbol_id = 66;
  buy_order.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder sell_order{};

  sell_order.order_id = 2;
  sell_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order.user_id = 20;
  sell_order.amount = 90;
  sell_order.price = 900;
  sell_order.symbol_id = 66;
  sell_order.trading_side = match_engine::TRADING_SITE_SELL;

  self->request(match_actor, caf::infinite, result_actor)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  std::this_thread::sleep_for(std::chrono::seconds(1));

  CAF_CHECK(1 == check_result.result_data_.size());
  CAF_CHECK(1000 == check_result.result_data_[0].price);
  CAF_CHECK(1 == check_result.result_data_[0].maker_id);
  CAF_CHECK(2 == check_result.result_data_[0].taker_id);
  CAF_CHECK(90 == check_result.result_data_[0].amount);
  CAF_CHECK(20 == check_result.result_data_[0].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[0].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[0].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_SELL ==
            check_result.result_data_[0].trading_side);
}

CAF_TEST(match_one_maker_tow_taker_order) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  CheckResult check_result;
  auto result_actor =
      sys.spawn(match_engine::MatchResultDealActor, &check_result);

  match_engine::RawOrder buy_order{};

  buy_order.order_id = 1;
  buy_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order.user_id = 10;
  buy_order.amount = 100;
  buy_order.price = 1000;
  buy_order.symbol_id = 66;
  buy_order.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder sell_order1{};

  sell_order1.order_id = 2;
  sell_order1.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order1.user_id = 20;
  sell_order1.amount = 50;
  sell_order1.price = 900;
  sell_order1.symbol_id = 66;
  sell_order1.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder sell_order2{};

  sell_order2.order_id = 3;
  sell_order2.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order2.user_id = 30;
  sell_order2.amount = 40;
  sell_order2.price = 950;
  sell_order2.symbol_id = 66;
  sell_order2.trading_side = match_engine::TRADING_SITE_SELL;

  self->request(match_actor, caf::infinite, result_actor)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order1)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order2)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  std::this_thread::sleep_for(std::chrono::seconds(2));

  CAF_CHECK(2 == check_result.result_data_.size());
  CAF_CHECK(1000 == check_result.result_data_[0].price);
  CAF_CHECK(1 == check_result.result_data_[0].maker_id);
  CAF_CHECK(2 == check_result.result_data_[0].taker_id);
  CAF_CHECK(50 == check_result.result_data_[0].amount);
  CAF_CHECK(20 == check_result.result_data_[0].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[0].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[0].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_SELL ==
            check_result.result_data_[0].trading_side);

  CAF_CHECK(1000 == check_result.result_data_[1].price);
  CAF_CHECK(1 == check_result.result_data_[1].maker_id);
  CAF_CHECK(3 == check_result.result_data_[1].taker_id);
  CAF_CHECK(40 == check_result.result_data_[1].amount);
  CAF_CHECK(30 == check_result.result_data_[1].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[1].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[1].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_SELL ==
            check_result.result_data_[0].trading_side);
}

CAF_TEST(match_tow_maker_one_taker_order) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  CheckResult check_result;
  auto result_actor =
      sys.spawn(match_engine::MatchResultDealActor, &check_result);

  match_engine::RawOrder buy_order{};

  buy_order.order_id = 1;
  buy_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order.user_id = 10;
  buy_order.amount = 100;
  buy_order.price = 1000;
  buy_order.symbol_id = 66;
  buy_order.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder sell_order1{};

  sell_order1.order_id = 2;
  sell_order1.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order1.user_id = 20;
  sell_order1.amount = 50;
  sell_order1.price = 900;
  sell_order1.symbol_id = 66;
  sell_order1.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder sell_order2{};

  sell_order2.order_id = 3;
  sell_order2.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order2.user_id = 30;
  sell_order2.amount = 40;
  sell_order2.price = 950;
  sell_order2.symbol_id = 66;
  sell_order2.trading_side = match_engine::TRADING_SITE_SELL;

  self->request(match_actor, caf::infinite, result_actor)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order1)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order2)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  std::this_thread::sleep_for(std::chrono::seconds(2));

  CAF_CHECK(2 == check_result.result_data_.size());
  CAF_CHECK(900 == check_result.result_data_[0].price);
  CAF_CHECK(2 == check_result.result_data_[0].maker_id);
  CAF_CHECK(1 == check_result.result_data_[0].taker_id);
  CAF_CHECK(50 == check_result.result_data_[0].amount);
  CAF_CHECK(20 == check_result.result_data_[0].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[0].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[0].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);

  CAF_CHECK(950 == check_result.result_data_[1].price);
  CAF_CHECK(3 == check_result.result_data_[1].maker_id);
  CAF_CHECK(1 == check_result.result_data_[1].taker_id);
  CAF_CHECK(40 == check_result.result_data_[1].amount);
  CAF_CHECK(30 == check_result.result_data_[1].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[1].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[1].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);
}

CAF_TEST(match_tow_maker_one_taker_later_maker_price_better_order) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  CheckResult check_result;
  auto result_actor =
      sys.spawn(match_engine::MatchResultDealActor, &check_result);

  match_engine::RawOrder buy_order{};

  buy_order.order_id = 1;
  buy_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order.user_id = 10;
  buy_order.amount = 100;
  buy_order.price = 1000;
  buy_order.symbol_id = 66;
  buy_order.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder sell_order1{};

  sell_order1.order_id = 2;
  sell_order1.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order1.user_id = 20;
  sell_order1.amount = 50;
  sell_order1.price = 900;
  sell_order1.symbol_id = 66;
  sell_order1.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder sell_order2{};

  sell_order2.order_id = 3;
  sell_order2.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order2.user_id = 30;
  sell_order2.amount = 40;
  sell_order2.price = 950;
  sell_order2.symbol_id = 66;
  sell_order2.trading_side = match_engine::TRADING_SITE_SELL;

  self->request(match_actor, caf::infinite, result_actor)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order2)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order1)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  std::this_thread::sleep_for(std::chrono::seconds(2));

  CAF_CHECK(2 == check_result.result_data_.size());
  CAF_CHECK(900 == check_result.result_data_[0].price);
  CAF_CHECK(2 == check_result.result_data_[0].maker_id);
  CAF_CHECK(1 == check_result.result_data_[0].taker_id);
  CAF_CHECK(50 == check_result.result_data_[0].amount);
  CAF_CHECK(20 == check_result.result_data_[0].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[0].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[0].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);

  CAF_CHECK(950 == check_result.result_data_[1].price);
  CAF_CHECK(3 == check_result.result_data_[1].maker_id);
  CAF_CHECK(1 == check_result.result_data_[1].taker_id);
  CAF_CHECK(40 == check_result.result_data_[1].amount);
  CAF_CHECK(30 == check_result.result_data_[1].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[1].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[1].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);
}

CAF_TEST(match_tow_maker_one_taker_maker_same_price_diff_time) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  CheckResult check_result;
  auto result_actor =
      sys.spawn(match_engine::MatchResultDealActor, &check_result);

  match_engine::RawOrder buy_order{};

  buy_order.order_id = 1;
  buy_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order.user_id = 10;
  buy_order.amount = 100;
  buy_order.price = 1000;
  buy_order.symbol_id = 66;
  buy_order.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder sell_order1{};

  sell_order1.order_id = 2;
  sell_order1.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order1.user_id = 20;
  sell_order1.amount = 50;
  sell_order1.price = 950;
  sell_order1.symbol_id = 66;
  sell_order1.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder sell_order2{};

  sell_order2.order_id = 3;
  sell_order2.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order2.user_id = 30;
  sell_order2.amount = 40;
  sell_order2.price = 950;
  sell_order2.symbol_id = 66;
  sell_order2.trading_side = match_engine::TRADING_SITE_SELL;

  self->request(match_actor, caf::infinite, result_actor)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order1)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order2)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  std::this_thread::sleep_for(std::chrono::seconds(2));

  CAF_CHECK(2 == check_result.result_data_.size());
  CAF_CHECK(950 == check_result.result_data_[0].price);
  CAF_CHECK(2 == check_result.result_data_[0].maker_id);
  CAF_CHECK(1 == check_result.result_data_[0].taker_id);
  CAF_CHECK(50 == check_result.result_data_[0].amount);
  CAF_CHECK(20 == check_result.result_data_[0].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[0].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[0].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);

  CAF_CHECK(950 == check_result.result_data_[1].price);
  CAF_CHECK(3 == check_result.result_data_[1].maker_id);
  CAF_CHECK(1 == check_result.result_data_[1].taker_id);
  CAF_CHECK(40 == check_result.result_data_[1].amount);
  CAF_CHECK(30 == check_result.result_data_[1].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[1].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[1].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);
}

CAF_TEST(match_three_maker_one_taker_maker) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  CheckResult check_result;
  auto result_actor =
      sys.spawn(match_engine::MatchResultDealActor, &check_result);

  match_engine::RawOrder buy_order{};

  buy_order.order_id = 1;
  buy_order.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order.user_id = 10;
  buy_order.amount = 100;
  buy_order.price = 1000;
  buy_order.symbol_id = 66;
  buy_order.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder sell_order1{};

  sell_order1.order_id = 2;
  sell_order1.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order1.user_id = 20;
  sell_order1.amount = 50;
  sell_order1.price = 950;
  sell_order1.symbol_id = 66;
  sell_order1.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder sell_order2{};

  sell_order2.order_id = 3;
  sell_order2.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order2.user_id = 30;
  sell_order2.amount = 40;
  sell_order2.price = 990;
  sell_order2.symbol_id = 66;
  sell_order2.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder sell_order3{};

  sell_order3.order_id = 4;
  sell_order3.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order3.user_id = 40;
  sell_order3.amount = 40;
  sell_order3.price = 960;
  sell_order3.symbol_id = 66;
  sell_order3.trading_side = match_engine::TRADING_SITE_SELL;

  self->request(match_actor, caf::infinite, result_actor)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order1)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order2)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order3)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  std::this_thread::sleep_for(std::chrono::seconds(2));

  CAF_CHECK(3 == check_result.result_data_.size());

  CAF_CHECK(950 == check_result.result_data_[0].price);
  CAF_CHECK(2 == check_result.result_data_[0].maker_id);
  CAF_CHECK(1 == check_result.result_data_[0].taker_id);
  CAF_CHECK(50 == check_result.result_data_[0].amount);
  CAF_CHECK(20 == check_result.result_data_[0].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[0].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[0].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);

  CAF_CHECK(960 == check_result.result_data_[1].price);
  CAF_CHECK(4 == check_result.result_data_[1].maker_id);
  CAF_CHECK(1 == check_result.result_data_[1].taker_id);
  CAF_CHECK(40 == check_result.result_data_[1].amount);
  CAF_CHECK(40 == check_result.result_data_[1].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[1].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[1].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);

  CAF_CHECK(990 == check_result.result_data_[2].price);
  CAF_CHECK(3 == check_result.result_data_[2].maker_id);
  CAF_CHECK(1 == check_result.result_data_[2].taker_id);
  CAF_CHECK(10 == check_result.result_data_[2].amount);
  CAF_CHECK(30 == check_result.result_data_[2].seller_user_id);
  CAF_CHECK(10 == check_result.result_data_[2].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[2].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);
}

CAF_TEST(match_complex) {
  auto match_actor = sys.spawn(match_engine::MatchActor);
  CheckResult check_result;
  auto result_actor =
      sys.spawn(match_engine::MatchResultDealActor, &check_result);

  match_engine::RawOrder sell_order1{};

  sell_order1.order_id = 1;
  sell_order1.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order1.user_id = 10;
  sell_order1.amount = 100;
  sell_order1.price = 900;
  sell_order1.symbol_id = 66;
  sell_order1.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder sell_order2{};

  sell_order2.order_id = 2;
  sell_order2.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order2.user_id = 20;
  sell_order2.amount = 100;
  sell_order2.price = 950;
  sell_order2.symbol_id = 66;
  sell_order2.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder buy_order3{};

  buy_order3.order_id = 3;
  buy_order3.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order3.user_id = 30;
  buy_order3.amount = 150;
  buy_order3.price = 1000;
  buy_order3.symbol_id = 66;
  buy_order3.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder buy_order4{};

  buy_order4.order_id = 4;
  buy_order4.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order4.user_id = 40;
  buy_order4.amount = 100;
  buy_order4.price = 960;
  buy_order4.symbol_id = 66;
  buy_order4.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder sell_order5{};

  sell_order5.order_id = 5;
  sell_order5.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order5.user_id = 50;
  sell_order5.amount = 100;
  sell_order5.price = 1200;
  sell_order5.symbol_id = 66;
  sell_order5.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder sell_order6{};

  sell_order6.order_id = 6;
  sell_order6.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  sell_order6.user_id = 60;
  sell_order6.amount = 100;
  sell_order6.price = 960;
  sell_order6.symbol_id = 66;
  sell_order6.trading_side = match_engine::TRADING_SITE_SELL;

  match_engine::RawOrder buy_order7{};

  buy_order7.order_id = 7;
  buy_order7.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order7.user_id = 70;
  buy_order7.amount = 50;
  buy_order7.price = 1200;
  buy_order7.symbol_id = 66;
  buy_order7.trading_side = match_engine::TRADING_SITE_BUY;

  match_engine::RawOrder buy_order8{};
  buy_order8.order_id = 8;
  buy_order8.submit_time =
      std::chrono::system_clock::now().time_since_epoch().count();
  buy_order8.user_id = 80;
  buy_order8.amount = 100;
  buy_order8.price = 1300;
  buy_order8.symbol_id = 66;
  buy_order8.trading_side = match_engine::TRADING_SITE_BUY;

  self->request(match_actor, caf::infinite, result_actor)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order1)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order2)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order3)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order4)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order5)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, sell_order6)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order7)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  self->request(match_actor, caf::infinite, buy_order8)
      .receive([=]() {}, [&](caf::error& err) { CAF_FAIL(sys.render(err)); });

  std::this_thread::sleep_for(std::chrono::seconds(2));

  CAF_CHECK(6 == check_result.result_data_.size());

  CAF_CHECK(900 == check_result.result_data_[0].price);
  CAF_CHECK(1 == check_result.result_data_[0].maker_id);
  CAF_CHECK(3 == check_result.result_data_[0].taker_id);
  CAF_CHECK(100 == check_result.result_data_[0].amount);
  CAF_CHECK(10 == check_result.result_data_[0].seller_user_id);
  CAF_CHECK(30 == check_result.result_data_[0].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[0].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[0].trading_side);

  CAF_CHECK(950 == check_result.result_data_[1].price);
  CAF_CHECK(2 == check_result.result_data_[1].maker_id);
  CAF_CHECK(3 == check_result.result_data_[1].taker_id);
  CAF_CHECK(50 == check_result.result_data_[1].amount);
  CAF_CHECK(20 == check_result.result_data_[1].seller_user_id);
  CAF_CHECK(30 == check_result.result_data_[1].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[1].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[1].trading_side);

  CAF_CHECK(950 == check_result.result_data_[2].price);
  CAF_CHECK(2 == check_result.result_data_[2].maker_id);
  CAF_CHECK(4 == check_result.result_data_[2].taker_id);
  CAF_CHECK(50 == check_result.result_data_[2].amount);
  CAF_CHECK(20 == check_result.result_data_[2].seller_user_id);
  CAF_CHECK(40 == check_result.result_data_[2].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[2].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[2].trading_side);

  CAF_CHECK(960 == check_result.result_data_[3].price);
  CAF_CHECK(4 == check_result.result_data_[3].maker_id);
  CAF_CHECK(6 == check_result.result_data_[3].taker_id);
  CAF_CHECK(50 == check_result.result_data_[3].amount);
  CAF_CHECK(60 == check_result.result_data_[3].seller_user_id);
  CAF_CHECK(40 == check_result.result_data_[3].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[3].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_SELL ==
            check_result.result_data_[3].trading_side);

  CAF_CHECK(960 == check_result.result_data_[4].price);
  CAF_CHECK(6 == check_result.result_data_[4].maker_id);
  CAF_CHECK(7 == check_result.result_data_[4].taker_id);
  CAF_CHECK(50 == check_result.result_data_[4].amount);
  CAF_CHECK(60 == check_result.result_data_[4].seller_user_id);
  CAF_CHECK(70 == check_result.result_data_[4].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[4].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[4].trading_side);

  CAF_CHECK(1200 == check_result.result_data_[5].price);
  CAF_CHECK(5 == check_result.result_data_[5].maker_id);
  CAF_CHECK(8 == check_result.result_data_[5].taker_id);
  CAF_CHECK(100 == check_result.result_data_[5].amount);
  CAF_CHECK(50 == check_result.result_data_[5].seller_user_id);
  CAF_CHECK(80 == check_result.result_data_[5].buyer_user_id);
  CAF_CHECK(66 == check_result.result_data_[5].symbol_id);
  CAF_CHECK(match_engine::TRADING_SITE_BUY ==
            check_result.result_data_[5].trading_side);
}

CAF_TEST_FIXTURE_SCOPE_END()
