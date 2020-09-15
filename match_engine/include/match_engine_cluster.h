/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef MATCH_ENGINE_INCLUDE_MATCH_ENGINE_CLUSTER_H_
#define MATCH_ENGINE_INCLUDE_MATCH_ENGINE_CLUSTER_H_

#include <cdcf/actor_system.h>

#include <caf/all.hpp>

#include "./match_engine_config.h"
namespace match_engine {
class MatchEngineCluster : public cdcf::cluster::Observer {
 public:
  void Update(const cdcf::cluster::Event& event) override;
  MatchEngineCluster(caf::actor_system& system, uint16_t symbolRouterPort,
                     uint16_t mergeResultPort, caf::actor symbolIdRouter,
                     std::string host);
  void Match(RawOrder raw_order);

  void AddNewNode(std::string host, std::string hostname, std::string role);

 private:
  caf::actor_system& system_;
  uint16_t symbol_router_port_;
  uint16_t merge_result_port_;
  caf::actor symbol_id_router_;
  caf::scoped_actor self_actor_;
  std::string host_;
};
}  // namespace match_engine

#endif  // MATCH_ENGINE_INCLUDE_MATCH_ENGINE_CLUSTER_H_
