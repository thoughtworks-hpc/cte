/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef CTE_MATCH_ENGINE_CLUSTER_H
#define CTE_MATCH_ENGINE_CLUSTER_H

#include <cdcf/actor_system.h>

#include <caf/all.hpp>

#include "match_engine_config.h"
namespace match_engine {
class MatchEngineCluster : public cdcf::cluster::Observer {
 public:
  void Update(const cdcf::cluster::Event& event) override;
  MatchEngineCluster(caf::actor_system& system, uint16_t symbolRouterPort,
                     uint16_t mergeResultPort,
                     caf::actor  symbolIdRouter);
  void Match(RawOrder raw_order);

 private:
  caf::actor_system& system_;
  uint16_t symbol_router_port_;
  uint16_t merge_result_port_;
  caf::actor symbol_id_router_;
  caf::scoped_actor self_actor_;
};
}

#endif  // CTE_MATCH_ENGINE_CLUSTER_H
