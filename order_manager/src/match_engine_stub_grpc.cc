/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/match_engine_stub_grpc.h"

#include <cdcf/logger.h>

#include <thread>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

MatchEngineStubGrpc::MatchEngineStubGrpc(
    const std::shared_ptr<Channel> &main_channel,
    const std::vector<std::shared_ptr<Channel>> &request_channel)
    : request_stub_index_(0) {
  if (main_channel != nullptr) {
    main_stub_ = ::match_engine_proto::TradingEngine::NewStub(main_channel);
    request_stubs_.push_back(main_stub_);
  }

  for (const auto &channel : request_channel) {
    request_stubs_.emplace_back(
        ::match_engine_proto::TradingEngine::NewStub(channel));
  }
}

::grpc::Status MatchEngineStubGrpc::Match(
    const match_engine_proto::Order &request,
    ::match_engine_proto::Reply *response) {
  grpc::Status status;

  if (!request_stubs_.empty()) {
    grpc::ClientContext client_context;
    status = GetNextRequestStub()->Match(&client_context, request, response);
    // CDCF_LOGGER_DEBUG("send match request for order {}", request.order_id());
  }

  return status;
}

std::shared_ptr<std::thread> MatchEngineStubGrpc::SubscribeMatchResult(
    std::function<void(::match_engine_proto::Trade)> handler) {
  std::shared_ptr<std::thread> t =
      std::make_shared<std::thread>([this, handler] {
        if (main_stub_ != nullptr) {
          grpc::ClientContext client_context;
          match_engine_proto::Trade trade;
          std::unique_ptr<ClientReader<match_engine_proto::Trade>> reader(
              main_stub_->SubscribeMatchResult(&client_context,
                                               google::protobuf::Empty()));
          while (reader->Read(&trade)) {
            // trade.PrintDebugString();
            handler(trade);
          }
          Status status = reader->Finish();
        }
      });

  return t;
}

std::shared_ptr<::match_engine_proto::TradingEngine::Stub>
MatchEngineStubGrpc::GetNextRequestStub() {
  std::lock_guard<std::mutex> lock(request_stubs_mutex_);
  assert(!request_stubs_.empty());

  if (request_stub_index_ != 0 &&
      request_stub_index_ == request_stubs_.size()) {
    request_stub_index_ = 0;
  }

  return request_stubs_[request_stub_index_++];
}
