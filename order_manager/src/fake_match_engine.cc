/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/fake_match_engine.h"

#include <thread>

::grpc::Status FakeMatchEngineService::Match(
    ::grpc::ServerContext *context, const ::match_engine_proto::Order *request,
    ::match_engine_proto::Reply *response) {
  response->set_status(match_engine_proto::STATUS_SUCCESS);
  response->set_message("all good");

  return ::grpc::Status(::grpc::StatusCode::OK, "");
}

::grpc::Status FakeMatchEngineService::SubscribeMatchResult(
    ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
    ::grpc::ServerWriter< ::match_engine_proto::Trade> *writer) {
  match_engine_proto::Trade trade;
  trade.set_amount(1);
  trade.set_maker_id(1);
  trade.set_taker_id(2);
  int count_down = 10;

  std::this_thread::sleep_for(std::chrono::seconds(20));

  while (count_down--) {
    writer->Write(trade);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return ::grpc::Status(::grpc::StatusCode::OK, "");
}
