/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef MATCH_ENGINE_INCLUDE_MATCH_ACTOR_H_
#define MATCH_ENGINE_INCLUDE_MATCH_ACTOR_H_

#include <list>
#include <vector>

#include <caf/io/all.hpp>

#include "./match_engine_config.h"

namespace match_engine {

using RawOrderList = std::list<RawOrder>;
using OrderTable = std::list<RawOrderList>;

struct OrderBook {
  OrderTable sell_list;
  OrderTable buy_list;
  caf::actor match_result_actor = nullptr;
};

caf::behavior MatchActor(caf::stateful_actor<OrderBook>* self);

}  // namespace match_engine

#endif  // MATCH_ENGINE_INCLUDE_MATCH_ACTOR_H_
