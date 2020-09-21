/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/symbol_id_router.h"

#include "../include/match_engine_config.h"
#include "cdcf/logger.h"

namespace match_engine {
caf::behavior SymbolRouterActor(caf::stateful_actor<SymbolRouterData>* self) {
  return {
      [=](int32_t symbol_id, const std::string& node_host,
          const caf::actor& actor) {
        // add new symbol actor of other node
        CDCF_LOGGER_INFO("Add remote symbol actor, symbol_id:{}, node_host:{}",
                         symbol_id, node_host);
        self->state.symbol_actor_map[symbol_id] = actor;
        self->state.node_symbol_map[node_host].push_back(symbol_id);
      },
      [=](int32_t symbol_id, const caf::actor& actor) {
        // add new symbol actor of localhost
        CDCF_LOGGER_INFO("Add local symbol actor, symbol_id:{}", symbol_id);
        self->state.local_symbol_actors.emplace_back(symbol_id, actor);
        self->state.symbol_actor_map[symbol_id] = actor;
      },
      [=](std::string& node_host) {
        // remove symbol actor of other node
        for (auto symbol_id : self->state.node_symbol_map[node_host]) {
          self->state.symbol_actor_map.erase(symbol_id);
        }
        self->state.node_symbol_map.erase(node_host);
      },
      [=](GetAtom) {
        // get local symbol list;
        std::string local_symbol;
        for (auto& one_actor : self->state.local_symbol_actors) {
          local_symbol += " " + std::to_string(one_actor.symbol_id);
        }
        CDCF_LOGGER_INFO("Receive symbol list query, return local symbol:{}",
                         local_symbol);
        return self->state.local_symbol_actors;
      },
      [=](caf::actor& merge_result_actor) {
        // set merge result actor to local
        for (auto& local_symbol_actor : self->state.local_symbol_actors) {
          self->send(local_symbol_actor.symbol_actor, merge_result_actor);
        }
      },
      [=](RawOrder msg) {
        if (0 == self->state.symbol_actor_map.count(msg.symbol_id)) {
          CDCF_LOGGER_ERROR("Can find symbol actor, symbol id:{}",
                            msg.symbol_id);
          return false;
        }
        self->send(self->state.symbol_actor_map[msg.symbol_id], msg);
        return true;
      }};
}
}  // namespace match_engine
