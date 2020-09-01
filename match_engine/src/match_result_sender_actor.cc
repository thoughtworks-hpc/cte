/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/match_result_sender_actor.h"

namespace match_engine {
caf::behavior MatchResultDealActor(caf::event_based_actor*,
                                   SenderMatchInterface* match_engine_grpc) {
  return {[&match_engine_grpc](const TradeListMsg& match_result) {
    match_engine_grpc->SendMatchResult(match_result.data);
  }};
}
}  // namespace match_engine
