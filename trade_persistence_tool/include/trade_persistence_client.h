/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef TRADE_PERSISTENCE_TOOL_INCLUDE_TRADE_PERSISTENCE_CLIENT_H_
#define TRADE_PERSISTENCE_TOOL_INCLUDE_TRADE_PERSISTENCE_CLIENT_H_

#include <string>
#include <utility>
#include <vector>

#include "../../common/include/database_interface.hpp"
#include "./database_write_interface.h"

class TradePersistenceClient {
 public:
  TradePersistenceClient(database_interface::InfluxDB* database,
                         std::string tradeEngineAddress,
                         std::string databaseTableName)
      : database(database),
        trade_engine_address_(std::move(tradeEngineAddress)),
        database_table_name_(std::move(databaseTableName)) {}

  bool PersistTrades();

 private:
  database_interface::InfluxDB* database;
  std::string trade_engine_address_;
  std::string database_table_name_;
};

std::vector<std::string> ParseIpAddress(std::string address);

#endif  // TRADE_PERSISTENCE_TOOL_INCLUDE_TRADE_PERSISTENCE_CLIENT_H_
