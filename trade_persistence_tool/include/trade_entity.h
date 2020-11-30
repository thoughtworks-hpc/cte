/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef TRADE_PERSISTENCE_TOOL_INCLUDE_TRADE_ENTITY_H_
#define TRADE_PERSISTENCE_TOOL_INCLUDE_TRADE_ENTITY_H_

#include <string>

#include "../../common/include/database_interface.hpp"
#include "../../common/protobuf_gen/match_engine.pb.h"
#include "./uuid.h"

class TradeEntity {
 public:
  std::string buy_order_id_;
  std::string sell_order_id_;
  std::string symbol_id_;
  std::string trade_id_;
  std::string price_;
  std::string amount_;
  std::string sell_user_id_;
  std::string buy_user_id_;
  uint64_t submit_time;

  explicit TradeEntity(const match_engine_proto::Trade& trade);

  database_interface::entity to_entity();
};

#endif  // TRADE_PERSISTENCE_TOOL_INCLUDE_TRADE_ENTITY_H_
