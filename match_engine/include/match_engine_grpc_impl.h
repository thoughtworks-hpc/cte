/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef CTE_MATCH_ENGINE_GRPC_IMPL_H
#define CTE_MATCH_ENGINE_GRPC_IMPL_H

#include <grpcpp/server.h>

#include <caf/all.hpp>

#include "match_engine_cluster.h"
#include "match_engine_config.h"

namespace match_engine {

class MatchEngineGRPCImpl final : public match_engine_proto::TradingEngine::Service, public SenderMatchInterface {
 public:
  grpc::Status Match(::grpc::ServerContext *context,
                     const ::match_engine_proto::Order *request,
                     ::match_engine_proto::Reply *response) override;
  grpc::Status SubscribeMatchResult(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::grpc::ServerWriter<::match_engine_proto::Trade> *writer) override;

  void Run();
  void RunWithWait();
  void SendMatchResult(const TradeList &trade_list) override;
  MatchEngineGRPCImpl(uint64_t server_port,
                      MatchEngineCluster &match_engine_cluster, bool is_test = false);

 private:
  uint64_t server_port_;
  std::unique_ptr<grpc::Server> server_;
  MatchEngineCluster &match_engine_cluster_;
  MatchResultWriteKeepers match_result_writer_keepers{};
  bool is_test_ = false;
};

}  // namespace match_engine

#endif  // CTE_MATCH_ENGINE_GRPC_IMPL_H
