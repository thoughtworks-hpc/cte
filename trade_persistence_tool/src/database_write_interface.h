/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_SRC_DATABASE_INTERFACE_H_
#define REQUEST_GENERATOR_SRC_DATABASE_INTERFACE_H_

#include <string>

#include "../../common/protobuf_gen/match_engine.pb.h"

class DatabaseWriteInterface {
 public:
  virtual bool PersistTrade(const match_engine_proto::Trade& trade, std::string uuid) = 0;
};

#endif  // REQUEST_GENERATOR_SRC_DATABASE_INTERFACE_H_
