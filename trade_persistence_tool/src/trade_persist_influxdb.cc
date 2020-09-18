/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "include/trade_persist_influxdb.h"

#include <cdcf/logger.h>

#include <string>

#include "../../common/include/influxdb.hpp"

bool TradePersistInfluxdb::PersistTrade(TradeEntity& trade) {
  influxdb_cpp::server_info si(ip_, std::stoi(port_), database_name_, username_,
                               password_);
  CDCF_LOGGER_DEBUG(
      "Connecting database server {}:{} with username:\"{}\" , "
      "password:\"{}\" ",
      ip_, port_, username_, password_);
  CDCF_LOGGER_DEBUG("Using database_name:\"{}\" , table_name:\"{}\"   ",
                    database_name_, database_table_name_);
  using time_stamp = std::chrono::time_point<std::chrono::system_clock,
                                             std::chrono::nanoseconds>;
  time_stamp current_time_stamp =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now());
  int64_t nanoseconds_since_epoch =
      current_time_stamp.time_since_epoch().count();

  std::string resp;
  int ret = influxdb_cpp::builder()
                .meas(database_table_name_)
                .tag("buy_order_id", trade.buy_order_id_)
                .tag("sell_order_id", trade.sell_order_id_)
                .field("symbol_id", trade.symbol_id_)
                .field("trade_id", trade.trade_id_)
                .field("price", trade.price_)
                .field("amount", trade.amount_)
                .field("sell_user_id", trade.sell_user_id_)
                .field("buy_user_id", trade.buy_user_id_)
                .field("deal_time", nanoseconds_since_epoch)
                .timestamp(trade.submit_time)
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    CDCF_LOGGER_DEBUG("Write db:");
    CDCF_LOGGER_DEBUG("buy_order_id: {}", trade.buy_order_id_);
    CDCF_LOGGER_DEBUG("sell_order_id: {}", trade.sell_order_id_);
    CDCF_LOGGER_DEBUG("symbol_id: {}", trade.symbol_id_);
    CDCF_LOGGER_DEBUG("trade_id: {}", trade.trade_id_);
    CDCF_LOGGER_DEBUG("price: {}", trade.price_);
    CDCF_LOGGER_DEBUG("amount: {}", trade.amount_);
    CDCF_LOGGER_DEBUG("sell_user_id: {}", trade.sell_user_id_);
    CDCF_LOGGER_DEBUG("buy_user_id: {}", trade.buy_user_id_);
    return true;
  } else {
    CDCF_LOGGER_ERROR("  Write db failed, ret:{} resp:{}", ret, resp);
    return false;
  }
}

int TradePersistInfluxdb::CreateDatabase() {
  std::string resp;
  int ret;
  influxdb_cpp::server_info si(ip_, std::stoi(port_), database_name_, username_,
                               password_);
  ret = influxdb_cpp::create_db(resp, database_name_, si);
  if (0 != ret) {
    CDCF_LOGGER_ERROR("Creat db failed ret:{}", ret);
  } else {
    CDCF_LOGGER_INFO("Creat {} database successfully ret:{}", database_name_,
                     ret);
  }
  return ret;
}
