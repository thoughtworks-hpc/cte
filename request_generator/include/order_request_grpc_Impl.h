//
// Created by Yuecheng Pei on 2020/8/27.
//

#ifndef CTE_ORDERREQUESTGRPCIMPL_H
#define CTE_ORDERREQUESTGRPCIMPL_H

namespace cte {
class OrderRequestGrpcImpl final : public ::OrderManager::Service {
 public:
  ::grpc::Status PlaceOrder(::grpc::ServerContext* context,
                            const ::order_manager_proto::Order* request,
                            ::order_manager_proto::Reply* response);
};
}  // namespace cte

#endif  // CTE_ORDERREQUESTGRPCIMPL_H
