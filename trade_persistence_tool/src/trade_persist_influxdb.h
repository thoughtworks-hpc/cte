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

  TradePersistInfluxdb(std::string databaseName, std::string ip,
                       std::string port)
      : database_name_(std::move(databaseName)),
        ip_(std::move(ip)),
        port_(std::move(port)) {
    CreateDatabase();
  }

  bool PersistTrade(const match_engine_proto::Trade& trade,
                    std::string uuid) override;

 private:
  int CreateDatabase();
};

#endif  // TRADE_PERSISTENCE_TOOL_SRC_TRADE_PERSIST_INFLUXDB_H_
