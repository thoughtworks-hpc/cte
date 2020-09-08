/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef TRADE_PERSISTENCE_TOOL_SRC_TRADE_PERSIST_INFLUXDB_H_
#define TRADE_PERSISTENCE_TOOL_SRC_TRADE_PERSIST_INFLUXDB_H_

#include <string>
#include <utility>

#include "./database_write_interface.h"

class TradePersistInfluxdb : public DatabaseWriteInterface {
 public:
  std::string database_name_;
  std::string ip_;
  std::string port_;
  std::string username_;
  std::string password_;

  TradePersistInfluxdb(std::string databaseName, std::string ip,
                       std::string port, std::string username,
                       std::string password)
      : database_name_(std::move(databaseName)),
        ip_(std::move(ip)),
        port_(std::move(port)),
        username_(std::move(username)),
        password_(std::move(password)) {
    CreateDatabase();
  }

  bool PersistTrade(TradeEntity& trade) override;

 private:
  int CreateDatabase();
};

#endif  // TRADE_PERSISTENCE_TOOL_SRC_TRADE_PERSIST_INFLUXDB_H_
