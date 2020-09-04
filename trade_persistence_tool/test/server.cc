//
// Created by Yuecheng Pei on 2020/9/3.
//

#include "server.h"

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>

#include <thread>

::grpc::Status MatchEngineImpl::Match(
    ::grpc::ServerContext *context, const ::match_engine_proto::Order *request,
    ::match_engine_proto::Reply *response) {
  return ::grpc::Status(::grpc::StatusCode::OK, "");
}

::grpc::Status MatchEngineImpl::SubscribeMatchResult(
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
    std::cout << "send trade back:" << std::endl;
    std::cout << "amount: " << trade.amount() << std::endl;
    std::cout << "maker_id: " << trade.maker_id() << std::endl;
    std::cout << "taker_id: " << trade.taker_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
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
