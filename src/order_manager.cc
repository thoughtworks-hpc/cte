/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "order_manager.h"
::grpc::Status OrderManagerImpl::PlaceOrder(::grpc::ServerContext *context,
                                            const ::order_manager::Order *request,
                                            ::order_manager::Reply *response) {
  ClientContext client_context;
  match_engine::Order order;
  match_engine::Reply *reply;

  stub_->Match(&client_context, order, reply);
  return grpc::Status::OK;
}
