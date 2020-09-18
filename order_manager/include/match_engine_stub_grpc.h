/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_GRPC_H_
#define ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_GRPC_H_

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <memory>
#include <vector>

#include "./match_engine_stub.h"

class MatchEngineStubGrpc : public MatchEngineStub {
 public:
  MatchEngineStubGrpc(
      const std::shared_ptr<grpc::Channel>& main_channel,
      const std::vector<std::shared_ptr<grpc::Channel>>& request_channel);

  int Match(const ::match_engine_proto::Order& request,
            ::match_engine_proto::Reply* response) override;
  void SubscribeMatchResult(
      std::function<void(::match_engine_proto::Trade)> handler) override;

 private:
  std::shared_ptr<::match_engine_proto::TradingEngine::Stub>
  GetNextRequestStub();

  //    grpc::ClientContext client_context_;

  std::shared_ptr<::match_engine_proto::TradingEngine::Stub> main_stub_;
  std::vector<std::shared_ptr<::match_engine_proto::TradingEngine::Stub>>
      request_stubs_;
  std::atomic<unsigned int> request_stub_index_;
  mutable std::mutex request_stubs_mutex_;
};

#endif  // ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_GRPC_H_
