/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/match_engine_stub_fake.h"

void MatchEngineStubFake::SubscribeMatchResult(
    std::function<void(::match_engine_proto::Trade)> handler) {
  handler_ = handler;
}

void MatchEngineStubFake::ProvideMatchResult(
    const match_engine_proto::Trade& trade) {
  handler_(trade);
}
