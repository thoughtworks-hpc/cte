/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_H_
#define ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_H_

#include <memory>
#include <thread>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"

class MatchEngineStub {
 public:
  virtual ::grpc::Status Match(const ::match_engine_proto::Order& request,
                               ::match_engine_proto::Reply* response) = 0;
  virtual std::shared_ptr<std::thread> SubscribeMatchResult(
      std::function<void(::match_engine_proto::Trade)> handler) = 0;
};

#endif  // ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_H_
