/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/match_engine_stub_fake.h"

std::shared_ptr<std::thread> MatchEngineStubFake::SubscribeMatchResult(
    std::function<void(::match_engine_proto::Trade)> handler) {
  handler_ = handler;
  return nullptr;
}

void MatchEngineStubFake::ProvideMatchResult(
    const match_engine_proto::Trade& trade) {
  handler_(trade);
}
