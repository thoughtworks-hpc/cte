/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "src/trade_persist_influxdb.h"

#include <cdcf/logger.h>

#include <string>

#include "../../common/include/influxdb.hpp"

bool TradePersistInfluxdb::PersistTrade(TradeEntity trade) {
  influxdb_cpp::server_info si(ip_, std::stoi(port_), database_name_, username_,
                               password_);
  std::string resp;

  int ret = influxdb_cpp::builder()
                .meas("trades")
                .tag("buy_trade_id", trade.buy_trade_id_)
                .tag("sell_trade_id", trade.sell_trade_id_)
                .field("symbol_id", trade.symbol_id_)
                .field("trade_id", trade.uuid_)
                .field("price", trade.price_)
                .field("amount", trade.amount_)
                .field("sell_user_id", trade.sell_user_id_)
                .field("buy_user_id", trade.buy_user_id_)
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    CDCF_LOGGER_DEBUG("  Write db success");
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
  influxdb_cpp::query(resp, "drop database " + database_name_, si);
  ret = influxdb_cpp::create_db(resp, database_name_, si);
  if (0 != ret) {
    CDCF_LOGGER_ERROR("Creat db failed ret:{}", ret);
  } else {
    CDCF_LOGGER_INFO("Creat {} database successfully ret:{}", database_name_,
                     ret);
  }
  return ret;
}
