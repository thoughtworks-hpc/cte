/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "src/trade_persistence_client.h"

#include <cdcf/logger.h>
#include <grpcpp/create_channel.h>

#include <memory>
#include <vector>

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
    CDCF_LOGGER_DEBUG("Receive #{} trade", ++count);
    CDCF_LOGGER_DEBUG("symbol_id: {}", trade.symbol_id());
    CDCF_LOGGER_DEBUG("maker_id: {}", trade.maker_id());
    CDCF_LOGGER_DEBUG("taker_id: {}", trade.taker_id());
    CDCF_LOGGER_DEBUG("price: {}", trade.price());
    CDCF_LOGGER_DEBUG("trading_side: {}", trade.trading_side());
    CDCF_LOGGER_DEBUG("amount: {}", trade.amount());
    CDCF_LOGGER_DEBUG("buyer_user_id: {}", trade.buyer_user_id());
    CDCF_LOGGER_DEBUG("seller_user_id: {}", trade.seller_user_id());

    auto trade_entity = TradeEntity(trade);
    if (!database->PersistTrade(trade_entity)) {
      CDCF_LOGGER_ERROR("  Write Database Failed");
      return false;
    }
  }
  CDCF_LOGGER_INFO("  Write Database succeed");
  ::grpc::Status status = reader->Finish();
  return true;
}
