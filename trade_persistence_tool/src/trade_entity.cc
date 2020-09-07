/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "src/trade_entity.h"

TradeEntity::TradeEntity(const match_engine_proto::Trade& trade) {
  buy_trade_id_ = std::to_string(trade.buyer_user_id());
  sell_trade_id_ = std::to_string(trade.seller_user_id());
  symbol_id_ = std::to_string(trade.symbol_id());
  trade_id_ = std::to_string(trade.symbol_id());
  price_ = trade.price();
  amount_ = trade.amount();
  trade_id_ = uuid::generate_uuid_v4();

  if (trade.trading_side() == ::match_engine_proto::TRADING_BUY) {
    buy_trade_id_ = std::to_string(trade.taker_id());
    sell_trade_id_ = std::to_string(trade.maker_id());
  }

  if (trade.trading_side() == ::match_engine_proto::TRADING_SELL) {
    sell_trade_id_ = std::to_string(trade.taker_id());
    buy_trade_id_ = std::to_string(trade.maker_id());
  }
}
