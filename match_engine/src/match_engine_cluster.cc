/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/match_engine_cluster.h"

#include <unistd.h>

#include <utility>

#include "cdcf/logger.h"
namespace match_engine {
MatchEngineCluster::MatchEngineCluster(caf::actor_system& system,
                                       uint16_t symbolRouterPort,
                                       uint16_t mergeResultPort,
                                       caf::actor symbolIdRouter,
                                       std::string host)
    : system_(system),
      self_actor_(system_),
      symbol_router_port_(symbolRouterPort),
      merge_result_port_(mergeResultPort),
      symbol_id_router_(std::move(symbolIdRouter)),
      host_(host) {}

void MatchEngineCluster::Update(const cdcf::cluster::Event& event) {
  if (event.member.hostname == host_ || event.member.host == host_) {
    CDCF_LOGGER_ERROR(
        "Receive locate node event ignore. hostname:{}, host:{}, member "
        "status:{} ",
        event.member.hostname, event.member.host, event.member.status);
    return;
  }

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

      AddNewNode(event.member.host, event.member.hostname, event.member.role);
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
void MatchEngineCluster::AddNewNode(std::string host, std::string hostname,
                                    std::string role) {
  if (host == host_ || hostname == host_) {
    CDCF_LOGGER_INFO("add locate node ignore. host:{},", host);
    return;
  }

  auto remote_symbol_router =
      system_.middleman().remote_actor(host, symbol_router_port_);

  for (int i = 0; i < 50; ++i) {
    if (!remote_symbol_router) {
      remote_symbol_router =
          system_.middleman().remote_actor(host, symbol_router_port_);
    } else {
      break;
    }
    CDCF_LOGGER_ERROR("Try to get remote symbol router {} times... ", i);
    sleep(1);
  }

  if (!remote_symbol_router) {
    CDCF_LOGGER_ERROR("Get remote symbol router failed, host:{}, port:{}", host,
                      symbol_router_port_);
    CDCF_LOGGER_ERROR("{}", caf::to_string(remote_symbol_router.error()));
  } else {
    CDCF_LOGGER_INFO("Get remote symbol router succeeded, host:{}, port:{}",
                     host, symbol_router_port_);
    self_actor_
        ->request(*remote_symbol_router, std::chrono::seconds(10),
                  GetAtom::value)
        .receive(
            [=](const std::vector<SymbolActorInfo>& symbol_actor_info_list) {
              caf::scoped_actor self(system_);
              for (const auto& symbol_actor_info : symbol_actor_info_list) {
                self->send(symbol_id_router_, symbol_actor_info.symbol_id, host,
                           symbol_actor_info.symbol_actor);
              }
            },
            [=](caf::error& err) {
              CDCF_LOGGER_ERROR(
                  "Get remote symbol info failed, host:{}, port:{}, err:{}",
                  host, symbol_router_port_, host, caf::to_string(err));
            });
  }

  if (role == kResultHostRoleName) {
    auto merge_result_actor_ptr =
        system_.middleman().remote_actor(host, merge_result_port_);

    for (int i = 0; i < 50; ++i) {
      if (!merge_result_actor_ptr) {
        merge_result_actor_ptr =
            system_.middleman().remote_actor(host, merge_result_port_);
      } else {
        break;
      }
      CDCF_LOGGER_ERROR("Try to get merge result actor {} times... ", i);
      sleep(1);
    }

    if (!merge_result_actor_ptr) {
      CDCF_LOGGER_ERROR(
          "Get remote merge result actor failed, remote host:{}, port:{}", host,
          merge_result_port_);
      CDCF_LOGGER_ERROR("{}", caf::to_string(merge_result_actor_ptr.error()));
    } else {
      CDCF_LOGGER_INFO(
          "get remote merge result host success. remote host:{}, port:{}", host,
          merge_result_port_);
      caf::actor merge_result_actor = *merge_result_actor_ptr;
      self_actor_->send(symbol_id_router_, merge_result_actor);
    }
  }
}

}  // namespace match_engine
