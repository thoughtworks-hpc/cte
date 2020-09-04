//
// Created by Yuecheng Pei on 2020/9/3.
//

#include "trade_persistence_client.h"

#include <grpcpp/create_channel.h>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"

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
    std::cout << "amount: " << trade.amount() << std::endl;
    std::cout << "maker_id: " << trade.maker_id() << std::endl;
    std::cout << "taker_id: " << trade.taker_id() << std::endl;
    if (!database->PersistTrade(trade)) {
      std::cout << "[ERROR] Write Database Failed" << std::endl;
      return false;
    };
  }
  ::grpc::Status status = reader->Finish();
  return true;
}
