/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef TRADE_PERSISTENCE_TOOL_INCLUDE_CONFIG_H_
#define TRADE_PERSISTENCE_TOOL_INCLUDE_CONFIG_H_

#include <cdcf/cdcf_config.h>
#include <cdcf/logger.h>

#include <mutex>
#include <string>
#include <vector>

#include "trade_entity.h"

extern std::vector<TradeEntity> trade_manager_db_buffer;
extern std::mutex trade_manager_db_buffer_mutex;

namespace trade_persistence_tool {
class Config {
 public:
  std::string db_ip;
  std::string db_port;
  std::string database_name;
  std::string database_table_name;
  std::string order_manager_address;
  std::string db_username;
  std::string db_password;
  cdcf::CDCFConfig cdcf_config;

  Config(int argc, char* argv[]);
};

std::vector<std::string> ParseIpAddress(std::string address);

}  // namespace trade_persistence_tool
#endif  // TRADE_PERSISTENCE_TOOL_INCLUDE_CONFIG_H_
