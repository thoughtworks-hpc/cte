/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "include/trade_persist_influxdb.h"

#include <cdcf/logger.h>

#include <string>

#include "../../common/include/influxdb.hpp"
#include "../include/config.h"

bool TradePersistInfluxdb::PersistTrade(TradeEntity& trade) {
  trade_manager_db_buffer_mutex.lock();
  trade_manager_db_buffer.emplace_back(trade);
  trade_manager_db_buffer_mutex.unlock();
  return true;

  influxdb_cpp::server_info si(ip_, std::stoi(port_), database_name_, username_,
                               password_);

  std::string resp;
  int ret = influxdb_cpp::builder()
                .meas(database_table_name_)
                .field("buy_order_id", trade.buy_order_id_)
                .field("sell_order_id", trade.sell_order_id_)
                .field("symbol_id", trade.symbol_id_)
                .field("trade_id", trade.trade_id_)
                .field("price", trade.price_)
                .field("amount", trade.amount_)
                .field("sell_user_id", trade.sell_user_id_)
                .field("buy_user_id", trade.buy_user_id_)
                .field("submit_time", static_cast<int64_t>(trade.submit_time))
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    CDCF_LOGGER_DEBUG(
        "Write #{}:  buy_order_id: {}, sell_order_id: {}, symbol_id: {}, "
        "trade_id: {}, "
        " price: {}, amount: {}, sell_user_id: {}, buy_user_id: {}",
        ++count_, trade.buy_order_id_, trade.sell_order_id_, trade.symbol_id_,
        trade.trade_id_, trade.price_, trade.amount_, trade.sell_user_id_,
        trade.buy_user_id_);

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
  CDCF_LOGGER_DEBUG(
      "Connecting database server {}:{} with username:\"{}\" , "
      "password:\"{}\" ",
      ip_, port_, username_, password_);
  CDCF_LOGGER_DEBUG("Using database_name:\"{}\" , table_name:\"{}\"   ",
                    database_name_, database_table_name_);
  ret = influxdb_cpp::create_db(resp, database_name_, si);
  if (0 != ret) {
    CDCF_LOGGER_ERROR("Creat db failed ret:{}", ret);
  } else {
    CDCF_LOGGER_INFO("Creat {} database successfully ret:{}", database_name_,
                     ret);
  }
  return ret;
}
