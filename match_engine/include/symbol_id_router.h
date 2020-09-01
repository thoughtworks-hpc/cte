/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef CTE_SYMBOL_ID_ROUTER_H
#define CTE_SYMBOL_ID_ROUTER_H

#include <iostream>
#include <map>
#include <vector>

#include <caf/all.hpp>
#include "match_engine_config.h"
namespace match_engine {



struct SymbolRouterData {
  std::map<int32_t, caf::actor> symbol_actor_map;
  std::map<std::string, std::vector<int32_t> > node_symbol_map;
  std::vector<SymbolActorInfo> local_symbol_actors;
};

caf::behavior SymbolRouterActor(caf::stateful_actor<SymbolRouterData>* self);
}  // namespace match_engine
#endif  // CTE_SYMBOL_ID_ROUTER_H
