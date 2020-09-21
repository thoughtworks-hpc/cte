/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <grpcpp/create_channel.h>
#include <gtest/gtest.h>

#include "../include/match_actor.h"
#include "../include/match_engine_cluster.h"
#include "../include/match_engine_config.h"
#include "../include/match_engine_grpc_impl.h"
#include "../include/match_result_sender_actor.h"
#include "../include/symbol_id_router.h"

const uint16_t k_match_result_port = 58900;
const uint16_t k_symbol_router_port = 58991;
const uint16_t k_match_engine_grpc = 58921;
const int32_t k_test_symbol_id1 = 66;
const int32_t k_test_symbol_id2 = 88;
const char* localhost_ = "127.0.0.1";

class MatchEngineGRPCTest : public ::testing::Test {
 protected:
  void SetUp() override {
    router_actor_ = system_.spawn(match_engine::SymbolRouterActor);
    match_actor1_ = system_.spawn(match_engine::MatchActor);

    self->send(router_actor_, k_test_symbol_id1, match_actor1_);
    self->send(router_actor_, k_test_symbol_id2, std::string(localhost_),
               match_actor2_);

    match_engine_cluster_ptr_ = new match_engine::MatchEngineCluster(
        system_, k_symbol_router_port, k_match_result_port, router_actor_,
        "localhost");

    match_engine_grpc_ = new match_engine::MatchEngineGRPCImpl(
        k_match_engine_grpc, *match_engine_cluster_ptr_, true);
    result_actor_ =
        system_.spawn(match_engine::MatchResultDealActor, match_engine_grpc_);
    self->send(router_actor_, result_actor_);
    match_engine_grpc_->Run();
  }

  void TearDown() override {
    delete match_engine_cluster_ptr_;
    delete match_engine_grpc_;
  }

 public:
  match_engine::Config config_;
  caf::actor_system system_{config_};
  caf::scoped_actor self{system_};
  caf::actor router_actor_;
  caf::actor match_actor1_;
  caf::actor match_actor2_;
  caf::actor result_actor_;
  match_engine::MatchEngineCluster* match_engine_cluster_ptr_{};
  match_engine::MatchEngineGRPCImpl* match_engine_grpc_{};
};

#if 1
TEST_F(MatchEngineGRPCTest, two_order_local_and_remote_path) {
  std::string grpc_host =
      std::string(localhost_) + ":" + std::to_string(k_match_engine_grpc);
  auto actor_system_channel =
      grpc::CreateChannel(grpc_host, grpc::InsecureChannelCredentials());

  ::match_engine_proto::TradingEngine::Stub client =
      ::match_engine_proto::TradingEngine::Stub(actor_system_channel);

  ::match_engine_proto::Order order1;
  match_engine_proto::Reply reply1;
  order1.set_order_id(1);
  order1.set_symbol_id(k_test_symbol_id1);
  order1.set_trading_side(::match_engine_proto::TRADING_BUY);
  order1.set_price(1000);
  order1.set_amount(100);
  order1.set_user_id(10);

  grpc::ClientContext match_context1;
  grpc::Status status = client.Match(&match_context1, order1, &reply1);

  if (!status.ok()) {
    ASSERT_TRUE(false);
    return;
  }

  EXPECT_EQ(::match_engine_proto::STATUS_SUCCESS, reply1.status());

  ::match_engine_proto::Order order2;
  match_engine_proto::Reply reply2;
  order2.set_order_id(1);
  order2.set_symbol_id(k_test_symbol_id2);
  order2.set_trading_side(::match_engine_proto::TRADING_BUY);
  order2.set_price(1000);
  order2.set_amount(100);
  order2.set_user_id(10);

  grpc::ClientContext match_context2;
  status = client.Match(&match_context2, order2, &reply2);

  if (!status.ok()) {
    ASSERT_TRUE(false);
    return;
  }

  EXPECT_EQ(::match_engine_proto::STATUS_SUCCESS, reply2.status());
}
#endif

TEST_F(MatchEngineGRPCTest, one_match) {
  std::string grpc_host =
      std::string(localhost_) + ":" + std::to_string(k_match_engine_grpc);
  auto actor_system_channel =
      grpc::CreateChannel(grpc_host, grpc::InsecureChannelCredentials());

  ::match_engine_proto::TradingEngine::Stub client =
      ::match_engine_proto::TradingEngine::Stub(actor_system_channel);

  grpc::ClientContext get_reader_context;
  auto match_result_reader_ptr =
      client.SubscribeMatchResult(&get_reader_context, {});

  ::match_engine_proto::Order order1;
  match_engine_proto::Reply reply1;
  order1.set_order_id(1);
  order1.set_symbol_id(k_test_symbol_id1);
  order1.set_trading_side(::match_engine_proto::TRADING_BUY);
  order1.set_price(1000);
  order1.set_amount(100);
  order1.set_user_id(10);

  grpc::ClientContext match_context1;
  grpc::Status status = client.Match(&match_context1, order1, &reply1);

  if (!status.ok()) {
    ASSERT_TRUE(false);
    return;
  }

  EXPECT_EQ(::match_engine_proto::STATUS_SUCCESS, reply1.status());

  ::match_engine_proto::Order order2;
  match_engine_proto::Reply reply2;
  order2.set_order_id(2);
  order2.set_symbol_id(k_test_symbol_id1);
  order2.set_trading_side(::match_engine_proto::TRADING_SELL);
  order2.set_price(990);
  order2.set_amount(200);
  order2.set_user_id(20);

  grpc::ClientContext match_context2;
  status = client.Match(&match_context2, order2, &reply2);

  if (!status.ok()) {
    ASSERT_TRUE(false);
    return;
  }

  // std::this_thread::sleep_for(std::chrono::seconds(10));

  EXPECT_EQ(::match_engine_proto::STATUS_SUCCESS, reply2.status());

  ::match_engine_proto::Trade trade;

  match_result_reader_ptr->Read(&trade);

  EXPECT_EQ(1, trade.maker_id());
  EXPECT_EQ(2, trade.taker_id());
  EXPECT_EQ(100, trade.amount());
  EXPECT_EQ(k_test_symbol_id1, trade.symbol_id());
  EXPECT_EQ(1000, trade.price());
  EXPECT_EQ(20, trade.seller_user_id());
  EXPECT_EQ(10, trade.buyer_user_id());
  EXPECT_EQ(::match_engine_proto::TRADING_SELL, trade.trading_side());
}

TEST_F(MatchEngineGRPCTest, should_not_match_order_when_engine_switch_close) {
  std::string grpc_host =
      std::string(localhost_) + ":" + std::to_string(k_match_engine_grpc);
  auto actor_system_channel =
      grpc::CreateChannel(grpc_host, grpc::InsecureChannelCredentials());

  ::match_engine_proto::TradingEngine::Stub client =
      ::match_engine_proto::TradingEngine::Stub(actor_system_channel);

  ::match_engine_proto::Order order1;
  order1.set_order_id(1);
  order1.set_symbol_id(k_test_symbol_id1);
  order1.set_trading_side(::match_engine_proto::TRADING_BUY);
  order1.set_price(1000);
  order1.set_amount(100);
  order1.set_user_id(10);

  ::match_engine_proto::EngineSwitch engine_status;
  engine_status.set_engine_status(::match_engine_proto::ENGINE_CLOSE);

  match_engine_proto::Reply reply1;
  match_engine_proto::Reply reply2;

  grpc::ClientContext match_context1;
  grpc::ClientContext match_context2;
  grpc::Status status1 =
      client.OpenCloseEngine(&match_context1, engine_status, &reply1);
  grpc::Status status2 = client.Match(&match_context2, order1, &reply2);

  if (status2.ok() || !status1.ok()) {
    ASSERT_TRUE(false);
    return;
  }

  EXPECT_EQ(::match_engine_proto::STATUS_SUCCESS, reply1.status());
}

TEST_F(MatchEngineGRPCTest, should_match_order_when_engine_switch_open) {
  std::string grpc_host =
      std::string(localhost_) + ":" + std::to_string(k_match_engine_grpc);
  auto actor_system_channel =
      grpc::CreateChannel(grpc_host, grpc::InsecureChannelCredentials());

  ::match_engine_proto::TradingEngine::Stub client =
      ::match_engine_proto::TradingEngine::Stub(actor_system_channel);

  ::match_engine_proto::Order order1;
  order1.set_order_id(1);
  order1.set_symbol_id(k_test_symbol_id1);
  order1.set_trading_side(::match_engine_proto::TRADING_BUY);
  order1.set_price(1000);
  order1.set_amount(100);
  order1.set_user_id(10);

  ::match_engine_proto::EngineSwitch engine_status;
  engine_status.set_engine_status(::match_engine_proto::ENGINE_OPEN);

  match_engine_proto::Reply reply1;
  match_engine_proto::Reply reply2;

  grpc::ClientContext match_context1;
  grpc::ClientContext match_context2;
  grpc::Status status1 =
      client.OpenCloseEngine(&match_context1, engine_status, &reply1);
  grpc::Status status2 = client.Match(&match_context2, order1, &reply2);

  if (!status2.ok() || !status1.ok()) {
    ASSERT_TRUE(false);
    return;
  }

  EXPECT_EQ(::match_engine_proto::STATUS_SUCCESS, reply1.status());
}

TEST_F(MatchEngineGRPCTest, engine_switch_from_close_to_open) {
  std::string grpc_host =
      std::string(localhost_) + ":" + std::to_string(k_match_engine_grpc);
  auto actor_system_channel =
      grpc::CreateChannel(grpc_host, grpc::InsecureChannelCredentials());

  ::match_engine_proto::TradingEngine::Stub client =
      ::match_engine_proto::TradingEngine::Stub(actor_system_channel);

  ::match_engine_proto::Order order1;
  order1.set_order_id(1);
  order1.set_symbol_id(k_test_symbol_id1);
  order1.set_trading_side(::match_engine_proto::TRADING_BUY);
  order1.set_price(1000);
  order1.set_amount(100);
  order1.set_user_id(10);

  ::match_engine_proto::EngineSwitch engine_status;
  engine_status.set_engine_status(::match_engine_proto::ENGINE_CLOSE);

  match_engine_proto::Reply reply1;
  match_engine_proto::Reply reply2;
  match_engine_proto::Reply reply3;
  match_engine_proto::Reply reply4;

  grpc::ClientContext match_context1;
  grpc::ClientContext match_context2;
  grpc::ClientContext match_context3;
  grpc::ClientContext match_context4;
  grpc::Status status1 =
      client.OpenCloseEngine(&match_context1, engine_status, &reply1);
  grpc::Status status2 = client.Match(&match_context2, order1, &reply2);

  if (status2.ok() || !status1.ok()) {
    ASSERT_TRUE(false);
    return;
  }

  EXPECT_EQ(::match_engine_proto::STATUS_SUCCESS, reply1.status());

  engine_status.set_engine_status(::match_engine_proto::ENGINE_OPEN);
  grpc::Status status3 =
      client.OpenCloseEngine(&match_context3, engine_status, &reply3);
  grpc::Status status4 = client.Match(&match_context4, order1, &reply4);

  if (!status3.ok() || !status4.ok()) {
    ASSERT_TRUE(false);
    return;
  }

  EXPECT_EQ(::match_engine_proto::STATUS_SUCCESS, reply3.status());
}
