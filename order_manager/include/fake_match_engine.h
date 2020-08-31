/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_FAKE_MATCH_ENGINE_H
#define CTE_FAKE_MATCH_ENGINE_H

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"

class FakeMatchEngineService
    : public match_engine_proto::TradingEngine::Service {
 public:
  ::grpc::Status Match(::grpc::ServerContext* context,
                       const ::match_engine_proto::Order* request,
                       ::match_engine_proto::Reply* response) override;
  ::grpc::Status SubscribeMatchResult(
      ::grpc::ServerContext* context, const ::google::protobuf::Empty* request,
      ::grpc::ServerWriter< ::match_engine_proto::Trade>* writer) override;
};

#endif  // CTE_FAKE_MATCH_ENGINE_H
