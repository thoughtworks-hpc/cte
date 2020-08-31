/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_ORDER_STORE_H
#define CTE_ORDER_STORE_H

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"

class OrderStore {
 public:
  virtual int PersistOrder(const match_engine_proto::Order &order,
                           std::string status) = 0;
};

#endif  // CTE_ORDER_STORE_H
