/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/match_result_sender_actor.h"

namespace match_engine {
caf::behavior MatchResultDealActor(caf::event_based_actor*,
                                   SenderMatchInterface* match_engine_grpc) {
  return {[=](const TradeListMsg& match_result) -> int {
    match_engine_grpc->SendMatchResult(match_result.data);
    return 0;
  }};
}
}  // namespace match_engine
