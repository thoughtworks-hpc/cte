/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef TRADE_PERSISTENCE_TOOL_SRC_DATABASE_WRITE_INTERFACE_H_
#define TRADE_PERSISTENCE_TOOL_SRC_DATABASE_WRITE_INTERFACE_H_

#include <string>

#include "../../common/protobuf_gen/match_engine.pb.h"

class DatabaseWriteInterface {
 public:
  virtual bool PersistTrade(const match_engine_proto::Trade& trade,
                            std::string uuid) = 0;
};

#endif  // TRADE_PERSISTENCE_TOOL_SRC_DATABASE_WRITE_INTERFACE_H_
