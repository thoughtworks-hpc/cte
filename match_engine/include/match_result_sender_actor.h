/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef MATCH_ENGINE_INCLUDE_MATCH_RESULT_SENDER_ACTOR_H_
#define MATCH_ENGINE_INCLUDE_MATCH_RESULT_SENDER_ACTOR_H_
#include <cdcf/actor_system.h>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"
#include "./match_engine_grpc_impl.h"

namespace match_engine {

caf::behavior MatchResultDealActor(caf::event_based_actor*,
                                   SenderMatchInterface* match_engine_grpc);
}  // namespace match_engine

#endif  // MATCH_ENGINE_INCLUDE_MATCH_RESULT_SENDER_ACTOR_H_
