/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef TRADE_PERSISTENCE_TOOL_TEST_SERVER_H_
#define TRADE_PERSISTENCE_TOOL_TEST_SERVER_H_
#include <grpcpp/server.h>

#include <memory>
#include <string>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"
#include "../../common/protobuf_gen/match_engine.pb.h"

class MatchEngineImpl final
    : public match_engine_proto::TradingEngine::Service {
 public:
  std::string port_;
  explicit MatchEngineImpl(const std::string &port);
  ::grpc::Status Match(::grpc::ServerContext *context,
                       const ::match_engine_proto::Order *request,
                       ::match_engine_proto::Reply *response) override;
  ::grpc::Status SubscribeMatchResult(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::grpc::ServerWriter< ::match_engine_proto::Trade> *writer) override;

  void Run();
  void RunWithWait();

 private:
  std::unique_ptr<grpc::Server> server_;
};

#endif  // TRADE_PERSISTENCE_TOOL_TEST_SERVER_H_
