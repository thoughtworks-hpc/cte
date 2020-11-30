/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "include/trade_persistence_client.h"

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
    // CDCF_LOGGER_DEBUG("Receive #{} trade", ++count);
    CDCF_LOGGER_DEBUG(
        "Receive #{} trade: symbol_id: {}, maker_id: {}, taker_id: {}, price: "
        "{}, "
        "trading_side: {}, amount: {}, buyer_user_id: {}, seller_user_id: {}",
        ++count, trade.symbol_id(), trade.maker_id(), trade.taker_id(),
        trade.price(), trade.trading_side(), trade.amount(),
        trade.buyer_user_id(), trade.seller_user_id());

    auto trade_entity = TradeEntity(trade).to_entity();
    if (!database->write(trade_entity)) {
      return false;
    } else {
      CDCF_LOGGER_DEBUG("Write #{} trade to database: {} success", count,
                        database_table_name_);
    }
  }
  CDCF_LOGGER_INFO("GRPC Reader finished");
  ::grpc::Status status = reader->Finish();
  return true;
}
