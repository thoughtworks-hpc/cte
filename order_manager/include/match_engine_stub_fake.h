/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_FAKE_H_
#define ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_FAKE_H_

#include "./match_engine_stub.h"

class MatchEngineStubFake : public MatchEngineStub {
 public:
  ::grpc::Status Match(const ::match_engine_proto::Order& request,
                       ::match_engine_proto::Reply* response) override {
    return ::grpc::Status::OK;
  }

  std::shared_ptr<std::thread> SubscribeMatchResult(
      std::function<void(::match_engine_proto::Trade)> handler) override;

  void ProvideMatchResult(const ::match_engine_proto::Trade& trade);

 private:
  std::function<void(::match_engine_proto::Trade)> handler_;
};

#endif  // ORDER_MANAGER_INCLUDE_MATCH_ENGINE_STUB_FAKE_H_
