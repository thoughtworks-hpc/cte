/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_INCLUDE_ORDER_REQUEST_GRPC_IMPL_H_
#define REQUEST_GENERATOR_INCLUDE_ORDER_REQUEST_GRPC_IMPL_H_



namespace cte {
class OrderRequestGrpcImpl final : public ::OrderManager::Service {
 public:
  ::grpc::Status PlaceOrder(::grpc::ServerContext* context,
                            const ::order_manager_proto::Order* request,
                            ::order_manager_proto::Reply* response);
};
}  // namespace cte

#endif  // REQUEST_GENERATOR_INCLUDE_ORDER_REQUEST_GRPC_IMPL_H_
