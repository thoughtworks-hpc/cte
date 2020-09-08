/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <gmock/gmock.h>

#include "src/config.h"
#include "src/trade_persistence_client.h"

using testing::Eq;

TEST(ParseIpAddress, should_parse_address_correctly_when_given_legal_input) {
  std::string address = "192.168.0.1:50051";
  std::string ip = trade_persistence_tool::ParseIpAddress(address)[0];
  std::string port = trade_persistence_tool::ParseIpAddress(address)[1];
  EXPECT_THAT(ip, Eq("192.168.0.1"));
  EXPECT_THAT(port, Eq("50051"));
}

TEST(TradeEntity, shoule_parse_buy_trade_side_correctly) {
  match_engine_proto::Trade trade;
  trade.set_maker_id(1);
  trade.set_taker_id(2);
  trade.set_trading_side(match_engine_proto::TradingSide::TRADING_BUY);

  TradeEntity trade_entity(trade);
  EXPECT_THAT(trade_entity.buy_order_id_, Eq("2"));
  EXPECT_THAT(trade_entity.sell_order_id_, Eq("1"));
}

TEST(TradeEntity, shoule_parse_sell_trade_side_correctly) {
  match_engine_proto::Trade trade;
  trade.set_maker_id(1);
  trade.set_taker_id(2);
  trade.set_trading_side(match_engine_proto::TradingSide::TRADING_SELL);

  TradeEntity trade_entity(trade);
  EXPECT_THAT(trade_entity.buy_order_id_, Eq("1"));
  EXPECT_THAT(trade_entity.sell_order_id_, Eq("2"));
}

TEST(TradeEntity, shoule_parse_all_properties_correctly) {
  match_engine_proto::Trade trade;
  trade.set_maker_id(1);
  trade.set_taker_id(2);
  trade.set_amount(10);
  trade.set_price(100);
  trade.set_buyer_user_id(1);
  trade.set_seller_user_id(2);
  trade.set_symbol_id(1);
  trade.set_trading_side(match_engine_proto::TradingSide::TRADING_SELL);

  TradeEntity trade_entity(trade);
  EXPECT_THAT(trade_entity.buy_order_id_, Eq("1"));
  EXPECT_THAT(trade_entity.sell_order_id_, Eq("2"));
  EXPECT_THAT(trade_entity.amount_, Eq("10"));
  EXPECT_THAT(trade_entity.price_, Eq("100"));
  EXPECT_THAT(trade_entity.buy_user_id_, Eq("1"));
  EXPECT_THAT(trade_entity.sell_user_id_, Eq("2"));
  EXPECT_THAT(trade_entity.symbol_id_, Eq("1"));
}
