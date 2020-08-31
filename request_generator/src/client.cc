/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <grpcpp/create_channel.h>

#include <iostream>

#include "../protobuf_gen/order.grpc.pb.h"
#include "../protobuf_gen/order.pb.h"

int main() {
  std::cout << "This is grpc client: " << std::endl;
  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());
  order_manager_proto::OrderManager::Stub clinet(channel);
  unsigned int seed = time(NULL);

  for (int i = 0; i < 5; i++) {
    grpc::ClientContext query_context;
    order_manager_proto::Order order;
    order.set_trading_side(order_manager_proto::TradingSide::TRADING_BUY);
    order_manager_proto::Reply reply;
    order.set_symbol(i);
    order.set_amount(rand_r(&seed) % 10);
    order.set_price(rand_r(&seed) % 100);
    order.set_user_id(i);
    grpc::Status status = clinet.PlaceOrder(&query_context, order, &reply);
    std::cout << "#" << i << " reply message: " << reply.message() << std::endl;
    std::cout << "#" << i << "reply error_code: " << reply.error_code()
              << std::endl;
  }
}
