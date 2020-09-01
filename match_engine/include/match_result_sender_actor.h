/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef CTE_MATCH_RESULT_SENDER_ACTOR_H
#define CTE_MATCH_RESULT_SENDER_ACTOR_H
#include <cdcf/actor_system.h>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"
#include "match_engine_grpc_impl.h"

namespace match_engine {

caf::behavior MatchResultDealActor(caf::event_based_actor*,
                                   SenderMatchInterface* match_engine_grpc);
}  // namespace match_engine

#endif  // CTE_MATCH_RESULT_SENDER_ACTOR_H
