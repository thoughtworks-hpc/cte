/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/match_engine_cluster.h"

#include <utility>

#include "cdcf/logger.h"
namespace match_engine {
MatchEngineCluster::MatchEngineCluster(caf::actor_system& system,
                                       uint16_t symbolRouterPort,
                                       uint16_t mergeResultPort,
                                       caf::actor symbolIdRouter)
    : system_(system),
      self_actor_(system_),
      symbol_router_port_(symbolRouterPort),
      merge_result_port_(mergeResultPort),
      symbol_id_router_(std::move(symbolIdRouter)) {}

void MatchEngineCluster::Update(const cdcf::cluster::Event& event) {
  // caf::scoped_actor self(system_);
  switch (event.member.status) {
    case cdcf::cluster::Member::Up: {
      CDCF_LOGGER_INFO(
          "Match engine member up, hostname:{}, host:{}, port:{}, name:{}, "
          "role:{}",
          event.member.hostname, event.member.host, event.member.port,
          event.member.name, event.member.role);
      break;
    }
    case cdcf::cluster::Member::Down: {
      CDCF_LOGGER_INFO(
          "Match engine member down, hostname:{}, host:{}, port:{}, name:{}, "
          "role:{}",
          event.member.hostname, event.member.host, event.member.port,
          event.member.name, event.member.role);
      break;
    }
    case cdcf::cluster::Member::ActorSystemDown: {
      CDCF_LOGGER_INFO(
          "Match engine member actor system down, hostname:{}, host:{}, "
          "port:{}, name:{}, "
          "role:{}",
          event.member.hostname, event.member.host, event.member.port,
          event.member.name, event.member.role);
      self_actor_->send(symbol_id_router_, event.member.host);
      break;
    }
    case cdcf::cluster::Member::ActorSystemUp: {
      CDCF_LOGGER_INFO(
          "Match engine member actor system up, hostname:{}, host:{}, port:{}, "
          "name:{}, "
          "role:{}",
          event.member.hostname, event.member.host, event.member.port,
          event.member.name, event.member.role);
      auto remote_symbol_router = system_.middleman().remote_actor(
          event.member.host, symbol_router_port_);

      if (!remote_symbol_router) {
        CDCF_LOGGER_ERROR("Get remote symbol router failed, host:{}, port:{}",
                          event.member.host, symbol_router_port_);
      } else {
        self_actor_
            ->request(*remote_symbol_router, std::chrono::seconds(10),
                      GetAtom::value)
            .receive(
                [=](const std::vector<SymbolActorInfo>&
                        symbol_actor_info_list) {
                  caf::scoped_actor self(system_);
                  for (const auto& symbol_actor_info : symbol_actor_info_list) {
                    self->send(symbol_id_router_, symbol_actor_info.symbol_id,
                               event.member.host,
                               symbol_actor_info.symbol_actor);
                  }
                },
                [=](caf::error& err) {
                  CDCF_LOGGER_ERROR(
                      "Get remote symbol info failed, host:{}, port:{}, err:{}",
                      event.member.host, symbol_router_port_, event.member.host,
                      caf::to_string(err));
                });
      }

      if (event.member.role == "merge_result_host") {
        auto merge_result_actor_ptr = system_.middleman().remote_actor(
            event.member.host, merge_result_port_);
        if (merge_result_actor_ptr) {
          CDCF_LOGGER_ERROR(
              "Get remote merge result actor failed, remote host:{}, port:{}",
              event.member.host, merge_result_port_);
        } else {
          CDCF_LOGGER_INFO(
              "get remote merge result host success. remote host:{}, port:{}",
              event.member.host, merge_result_port_);
          caf::actor merge_result_actor = *merge_result_actor_ptr;
          self_actor_->send(symbol_id_router_, merge_result_actor);
        }
      }
      break;
    }
    default:
      CDCF_LOGGER_ERROR("Can not handle unknown event:{}", event.member.status);
      break;
  }
}

void MatchEngineCluster::Match(RawOrder raw_order) {
  self_actor_->send(symbol_id_router_, raw_order);
}

}  // namespace match_engine
