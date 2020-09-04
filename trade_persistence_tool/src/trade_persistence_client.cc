/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "src/trade_persistence_client.h"

#include <google/protobuf/util/time_util.h>
#include <grpcpp/create_channel.h>

#include <memory>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"
#include "./uuid.h"

bool TradePersistenceClient::PersistTrades() {
  auto channel = grpc::CreateChannel(trade_engine_address_,
                                     grpc::InsecureChannelCredentials());
  ::match_engine_proto::TradingEngine::Stub clinet(channel);

  // std::this_thread::sleep_for(std::chrono::seconds(10));
  grpc::ClientContext client_context;
  match_engine_proto::Trade trade;
  std::unique_ptr<::grpc::ClientReader<match_engine_proto::Trade>> reader(
      clinet.SubscribeMatchResult(&client_context, google::protobuf::Empty()));
  int count = 0;
  while (reader->Read(&trade)) {
    // trade.PrintDebugString();
    std::cout << "receive #" << ++count << " trade:" << std::endl;
    //    std::cout << "amount: " << trade.amount() << std::endl;
    //    std::cout << "maker_id: " << trade.maker_id() << std::endl;
    //    std::cout << "taker_id: " << trade.taker_id() << std::endl;
    std::string uuid = uuid::generate_uuid_v4();
    std::cout << "uuid: " << uuid << std::endl;
    if (!database->PersistTrade(trade, uuid)) {
      std::cout << "[ERROR] Write Database Failed" << std::endl;
      return false;
    }
  }
  ::grpc::Status status = reader->Finish();
  return true;
}

// std::string TradePersistenceClient::GenerateTradeID(std::string seed) {
//  unsigned int hash = 0;
//  for (int i = 0; i < seed.size(); i++) {
//    // equivalent to: hash = 65599*hash + (*str++);
//    hash = seed[i] + (hash << 6) + (hash << 16) - hash;
//  }
//  std::cout << "hash_id: " << hash << std::endl;
//  return "(hash & 0x7FFFFFFF)";
//}
