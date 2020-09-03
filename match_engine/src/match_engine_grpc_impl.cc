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
          grpc_order.submit_time()) /
      1000000;

  return true;
}

grpc::Status match_engine::MatchEngineGRPCImpl::Match(
    ::grpc::ServerContext *context, const ::match_engine_proto::Order *request,
    ::match_engine_proto::Reply *response) {
  CDCF_LOGGER_INFO(
      "Receive Match request order, id:{}, symbol id: {}, user id:{}, "
      "price:{}, "
      "amount:{}, commit time:{}",
      request->order_id(), request->symbol_id(), request->user_id(),
      request->price(), request->amount(), request->submit_time().seconds());

  RawOrder raw_order{};
  bool can_transform = TransformGrpcOrder(*request, raw_order);

  if (!can_transform) {
    CDCF_LOGGER_ERROR("receive bad order data");
    return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION,
                          "data form error");
  }

  match_engine_cluster_.Match(raw_order);
  response->set_status(::match_engine_proto::STATUS_SUCCESS);

  return ::grpc::Status::OK;
}

grpc::Status match_engine::MatchEngineGRPCImpl::SubscribeMatchResult(
    ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
    ::grpc::ServerWriter< ::match_engine_proto::Trade> *writer) {
  CDCF_LOGGER_INFO("SubscribeMatchResult request");
  MatchResultWriterKeeper result_writer_keeper;
  result_writer_keeper.writer = writer;
  result_writer_keeper.writer_promise = new std::promise<int>();

  match_result_writer_keepers.push_back(result_writer_keeper);

  // waiting for writer end
  (void)result_writer_keeper.writer_promise->get_future().get();
  delete result_writer_keeper.writer_promise;
  return ::grpc::Status::OK;
}

void match_engine::MatchEngineGRPCImpl::Run() {
  std::string server_address("0.0.0.0:" + std::to_string(server_port_));
  CDCF_LOGGER_INFO("Match engine service up at port:{}", server_address);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(this);
  server_ = builder.BuildAndStart();
}

MatchEngineGRPCImpl::MatchEngineGRPCImpl(
    uint64_t server_port, MatchEngineCluster &match_engine_cluster,
    bool is_test)
    : server_port_(server_port),
      match_engine_cluster_(match_engine_cluster),
      is_test_(is_test) {}

void match_engine::MatchEngineGRPCImpl::RunWithWait() {
  Run();
  server_->Wait();
}

void TransformGrpcTrade(const MatchedTrade &trade,
                        ::match_engine_proto::Trade grpc_trade) {
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
}

void MatchEngineGRPCImpl::SendMatchResult(const TradeList &trade_list) {
  for (const auto &trade : trade_list) {
    CDCF_LOGGER_ERROR(
        "send merge trade to client: taker id:{}, maker id:{}, buyer:{}, "
        "seller:{}, deal time:{}, trade side:{}, price:{}, amount:{}",
        trade.taker_id, trade.maker_id, trade.buyer_user_id,
        trade.seller_user_id, trade.submit_time, trade.trading_side,
        trade.price, trade.amount);

    for (auto match_writer_it = match_result_writer_keepers.begin();
         match_writer_it != match_result_writer_keepers.end();) {
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

      bool success = (*match_writer_it).writer->Write(grpc_trade);

      if (!success) {
        match_writer_it->writer_promise->set_value(0);
        match_writer_it = match_result_writer_keepers.erase(match_writer_it);
        CDCF_LOGGER_WARN("A client is disconnect");
      } else {
        if (is_test_) {
          match_writer_it->writer_promise->set_value(0);
        }
        ++match_writer_it;
      }
    }
  }
}

}  // namespace match_engine
