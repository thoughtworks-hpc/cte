/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <grpcpp/server_builder.h>

#include <atomic>
#include <iostream>
#include <utility>

#include "../protobuf_gen/order.grpc.pb.h"
#include "../protobuf_gen/order.pb.h"

class OrderManagerImpl final
    : public order_manager_proto::OrderManager::Service {
 public:
  std::string port;
  std::atomic_int32_t count = 0;
  explicit OrderManagerImpl(std::string port) : port(std::move(port)) {}

  ::grpc::Status PlaceOrder(::grpc::ServerContext* context,
                            const ::order_manager_proto::Order* request,
                            ::order_manager_proto::Reply* response) {
    //    std::cout << "-------------------------------" << std::endl;
    std::cout << "Receive #" << ++count << " request from client" << std::endl;
    //    std::cout << "symbol: " << request->symbol() << std::endl;
    //    std::cout << "user_id: " << request->user_id() << std::endl;
    //    std::cout << "trading_side: " << request->trading_side() << std::endl;
    //    std::cout << "amount: " << request->amount() << std::endl;
    //    std::cout << "price: " << request->price() << std::endl;

    response->set_error_code(order_manager_proto::ErrorCode::FAILURE);
    std::string response_message =
        "#" + std::to_string(count) + " Response form server";
    response->set_message(response_message);

    return grpc::Status::OK;
  }

  void Run() {
    std::string server_address("0.0.0.0:" + port);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    server_ = builder.BuildAndStart();
    std::cout << "grpc server is listening " << port << std::endl;
  }

  void RunWithWait() {
    Run();
    server_->Wait();
  }

 private:
  std::unique_ptr<grpc::Server> server_;
};

int main(int argc, char* argv[]) {
  std::string port = "50052";
  if (strcmp(argv[1], "port") == 0) {
    port = argv[2];
  }
  std::cout << "This is grpc test" << std::endl;
  OrderManagerImpl order_manager_impl(port);
  order_manager_impl.RunWithWait();
}

void writeData(order_manager_proto::Order* order) { order->set_amount(10); }
