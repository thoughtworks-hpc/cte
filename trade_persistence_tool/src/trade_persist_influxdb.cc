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
