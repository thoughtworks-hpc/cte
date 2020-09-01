//
// Created by Mingfei Deng on 2020/8/25.
//

#include "../include/match_engine_grpc_impl.h"

#include <cdcf/logger.h>
#include <google/protobuf/util/time_util.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>

namespace match_engine {

bool TransformGrpcOrder(const ::match_engine_proto::Order &grpc_order,
                        RawOrder &raw_order) {
  raw_order.order_id = grpc_order.order_id();

  if (::match_engine_proto::TRADING_BUY == grpc_order.trading_side()) {
    raw_order.trading_side = TRADING_SITE_BUY;
  } else if (::match_engine_proto::TRADING_SELL == grpc_order.trading_side()) {
    raw_order.trading_side = TRADING_SITE_SELL;
  } else {
    CDCF_LOGGER_ERROR("transform unknown trading_side:{}",
                      grpc_order.trading_side());

    return false;
  }

  raw_order.symbol_id = grpc_order.symbol_id();
  raw_order.price = grpc_order.price();
  raw_order.amount = grpc_order.amount();
  raw_order.user_id = grpc_order.user_id();
  raw_order.submit_time =
      ::google::protobuf::util::TimeUtil::TimestampToMilliseconds(
          grpc_order.submit_time()) / 1000000;

  return true;
}

grpc::Status match_engine::MatchEngineGRPCImpl::Match(
    ::grpc::ServerContext *context, const ::match_engine_proto::Order *request,
    ::match_engine_proto::Reply *response) {
  CDCF_LOGGER_INFO("Receive Match request");
  RawOrder raw_order;
  bool can_transform = TransformGrpcOrder(*request, raw_order);

  if (!can_transform){
    CDCF_LOGGER_ERROR("receive bad order data");
    return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION, "data form error");
  }

  match_engine_cluster_.Match(raw_order);

  return ::grpc::Status::OK;
}
grpc::Status match_engine::MatchEngineGRPCImpl::SubscribeMatchResult(
    ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
    ::grpc::ServerWriter< ::match_engine_proto::Trade> *writer) {
  CDCF_LOGGER_INFO("SubscribeMatchResult request");
  match_writer_list_.push_back(writer);
  return ::grpc::Status::OK;
}
void match_engine::MatchEngineGRPCImpl::Run() {
  std::string server_address("0.0.0.0:" + std::to_string(server_port_));
  CDCF_LOGGER_INFO("Match engine service up at port:{}", server_port_);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(this);
  server_ = builder.BuildAndStart();
}

MatchEngineGRPCImpl::MatchEngineGRPCImpl(
    uint64_t server_port, MatchEngineCluster &match_engine_cluster)
    : server_port_(server_port), match_engine_cluster_(match_engine_cluster) {}

void match_engine::MatchEngineGRPCImpl::RunWithWait() {
  Run();
  server_->Wait();
}

::match_engine_proto::Trade TransformGrpcTrade(const MatchedTrade &trade) {
  ::match_engine_proto::Trade grpc_trade;

  grpc_trade.set_taker_id(trade.taker_id);
  grpc_trade.set_seller_user_id(trade.seller_user_id);
  grpc_trade.set_buyer_user_id(trade.buyer_user_id);
  grpc_trade.set_amount(trade.amount);
  grpc_trade.set_price(trade.price);
  grpc_trade.set_symbol_id(trade.symbol_id);
  grpc_trade.set_maker_id(trade.maker_id);

  if (TRADING_SITE_BUY == trade.trading_side) {
    grpc_trade.set_trading_side(::match_engine_proto::TRADING_BUY);
  } else {
    grpc_trade.set_trading_side(::match_engine_proto::TRADING_SELL);
  }

  auto grpc_timestamp =
      ::google::protobuf::util::TimeUtil::SecondsToTimestamp(trade.submit_time);
  grpc_trade.set_allocated_deal_time(&grpc_timestamp);

  return grpc_trade;
}

void MatchEngineGRPCImpl::SendMatchResult(const TradeList &trade_list) {
  for (auto match_writer_it = match_writer_list_.begin();
       match_writer_it != match_writer_list_.end();) {
    for (const auto &trade : trade_list) {
      bool success = (*match_writer_it)->Write(TransformGrpcTrade(trade));
      if (!success) {
        match_writer_it = match_writer_list_.erase(match_writer_it);
        CDCF_LOGGER_WARN("A client is disconnect");
      } else {
        ++match_writer_it;
      }
    }
  }
}

}  // namespace match_engine
