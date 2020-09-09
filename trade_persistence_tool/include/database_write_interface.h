/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef TRADE_PERSISTENCE_TOOL_INCLUDE_DATABASE_WRITE_INTERFACE_H_
#define TRADE_PERSISTENCE_TOOL_INCLUDE_DATABASE_WRITE_INTERFACE_H_

#include <string>

#include "../../common/protobuf_gen/match_engine.pb.h"
#include "./trade_entity.h"

class DatabaseWriteInterface {
 public:
  virtual bool PersistTrade(TradeEntity& trade) = 0;
};

#endif  // TRADE_PERSISTENCE_TOOL_INCLUDE_DATABASE_WRITE_INTERFACE_H_
