/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "test/fake_server.h"

#include <google/protobuf/util/time_util.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>

#include <string>
#include <thread>

::grpc::Status MatchEngineImpl::Match(
    ::grpc::ServerContext *context, const ::match_engine_proto::Order *request,
    ::match_engine_proto::Reply *response) {
  return ::grpc::Status(::grpc::StatusCode::OK, "");
}

::grpc::Status MatchEngineImpl::SubscribeMatchResult(
    ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
    ::grpc::ServerWriter< ::match_engine_proto::Trade> *writer) {
  int count_down = 5;
  while (count_down--) {
    match_engine_proto::Trade trade;
    trade.set_amount(1);
    trade.set_maker_id(1);
    trade.set_taker_id(2);
    trade.set_trading_side(match_engine_proto::TradingSide::TRADING_BUY);
    trade.set_buyer_user_id(1);
    trade.set_seller_user_id(2);
    trade.set_price(100);

    //    auto time = google::protobuf::util::TimeUtil::GetCurrentTime();
    //    trade.set_allocated_deal_time(&time);

    std::cout << "send #" << 5 - count_down << " trade back:" << std::endl;
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    writer->Write(trade);

    std::cout << "amount: " << trade.amount() << std::endl;
    std::cout << "maker_id: " << trade.maker_id() << std::endl;
    std::cout << "taker_id: " << trade.taker_id() << std::endl;
  }

  return ::grpc::Status(::grpc::StatusCode::OK, "");
}

void MatchEngineImpl::Run() {
  std::string server_address("0.0.0.0:" + port_);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(this);
  server_ = builder.BuildAndStart();
  std::cout << "grpc server is listening " << port_ << std::endl;
}

void MatchEngineImpl::RunWithWait() {
  Run();
  server_->Wait();
}
MatchEngineImpl::MatchEngineImpl(const std::string &port) : port_(port) {}
