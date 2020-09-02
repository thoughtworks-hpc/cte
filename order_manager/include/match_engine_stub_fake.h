/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_MATCH_ENGINE_STUB_FAKE_H
#define CTE_MATCH_ENGINE_STUB_FAKE_H

#include "./match_engine_stub.h"

class MatchEngineStubFake : public MatchEngineStub {
 public:
  int Match(const ::match_engine_proto::Order& request,
            ::match_engine_proto::Reply* response) override {
    return 0;
  }

  void SubscribeMatchResult(
      std::function<void(::match_engine_proto::Trade)> handler) override;

  void ProvideMatchResult(const ::match_engine_proto::Trade& trade);

 private:
  std::function<void(::match_engine_proto::Trade)> handler_;
};

#endif  // CTE_MATCH_ENGINE_STUB_FAKE_H
