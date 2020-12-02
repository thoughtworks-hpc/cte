/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_ORDER_STORE_H_
#define ORDER_MANAGER_INCLUDE_ORDER_STORE_H_

#include <string>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"
#include "./order.h"

class OrderStore {
 public:
  virtual int PersistOrder(const Order &order, std::string status,
                           int concluded_amount) = 0;
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_STORE_H_
