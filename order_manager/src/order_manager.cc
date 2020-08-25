/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_manager.h"
::grpc::Status OrderManagerImpl::PlaceOrder(::grpc::ServerContext *context,
                                            const ::order_manager_proto::Order *request,
                                            ::order_manager_proto::Reply *response) {
  ClientContext client_context;
  match_engine_proto::Order order;
  match_engine_proto::Reply *reply = nullptr;

  stub_->Match(&client_context, order, reply);
  return grpc::Status::OK;
}
