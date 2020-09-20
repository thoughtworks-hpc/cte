/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <regex>
#include <string>

#include <caf/all.hpp>

#include "../include/match_actor.h"
#include "../include/match_engine_cluster.h"
#include "../include/match_engine_grpc_impl.h"
#include "../include/match_result_sender_actor.h"
#include "../include/symbol_id_router.h"
#include "cdcf/logger.h"

std::vector<std::string> StrSplit(const std::string& in,
                                  const std::string& tag) {
  std::regex re{tag};

  return std::vector<std::string>{
      std::sregex_token_iterator(in.begin(), in.end(), re, -1),
      std::sregex_token_iterator()};
}

void caf_main(caf::actor_system& system, const match_engine::Config& config) {
  caf::actor router_actor = system.spawn(match_engine::SymbolRouterActor);
  system.middleman().publish(router_actor, config.match_router_port);
  match_engine::MatchEngineCluster match_engine_cluster(
      system, config.match_router_port, config.merge_result_port, router_actor,
      config.host_);

  match_engine::MatchEngineGRPCImpl match_engine_grpc(config.grpc_server_port,
                                                      match_engine_cluster);

  auto symbol_str_list = StrSplit(config.symbol_id_list, ",");

  caf::scoped_actor self(system);

  for (auto& symbol_str : symbol_str_list) {
    if (!symbol_str.empty()) {
      int32_t symbol_id = std::stoi(symbol_str);
      auto match_actor = system.spawn(match_engine::MatchActor);
      self->send(router_actor, symbol_id, match_actor);
    }
  }

  caf::actor result_merge_actor;

  if (match_engine::kResultHostRoleName == config.role_) {
    CDCF_LOGGER_INFO("spawn result actor, port:{}", config.merge_result_port);
    result_merge_actor =
        system.spawn(match_engine::MatchResultDealActor, &match_engine_grpc);
    system.middleman().publish(result_merge_actor, config.merge_result_port);
    self->send(router_actor, result_merge_actor);
  }
  cdcf::cluster::Cluster::GetInstance()->AddObserver(&match_engine_cluster);
  cdcf::cluster::Cluster::GetInstance()->NotifyReady();
  auto members = cdcf::cluster::Cluster::GetInstance()->GetMembers();

  for (auto& member : members) {
    match_engine_cluster.AddNewNode(member.host, member.hostname, member.role);
  }

  match_engine_grpc.RunWithWait();
}

CAF_MAIN(caf::io::middleman)
