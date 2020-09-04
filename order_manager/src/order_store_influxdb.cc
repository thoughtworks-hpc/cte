/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */
#include "../include/order_store_influxdb.h"

#include "../../common/include/influxdb.hpp"

int OrderStoreInfluxDB::PersistOrder(const match_engine_proto::Order& order,
                                     std::string status, int concluded_amount) {
  influxdb_cpp::server_info si("127.0.0.1", 8086, "order_manager", "", "");
  std::string resp;
  std::string trading_side =
      order.trading_side() == match_engine_proto::TRADING_BUY ? "buy" : "sell";

  int ret = influxdb_cpp::builder()
                .meas("order")
                .tag("order_id", std::to_string(order.order_id()))
                .tag("symbol_id", std::to_string(order.symbol_id()))
                .field("user_id", order.user_id())
                .field("price", order.price())
                .field("amount", order.amount())
                .field("trading_side", trading_side)
                .field("status", std::string(status))
                .field("concluded amount", concluded_amount)
                .timestamp(order.submit_time().seconds() * 1000000000 +
                           order.submit_time().nanos())
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    std::cout << "write db success" << std::endl;
  } else {
    std::cout << "write db failed, ret:" << ret << " resp:" << resp
              << std::endl;
  }

  return ret;
}
