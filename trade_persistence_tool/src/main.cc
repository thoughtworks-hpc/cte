/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <cdcf/logger.h>
#include <unistd.h>

#include "./trade_persist_influxdb.h"
#include "./trade_persistence_client.h"

int main(int argc, char* argv[]) {
  std::string db_ip;
  std::string db_port;
  std::string database_name;
  std::string order_manager_address;
  std::string db_username;
  std::string db_password;

  int option;
  while ((option = getopt(argc, argv, "m:d:n:l::u::p::")) != -1) {
    switch (option) {
      case 'm':
        order_manager_address = optarg;
        break;
      case 'd':
        db_ip = ParseIpAddress(optarg)[0];
        db_port = ParseIpAddress(optarg)[1];
        break;
      case 'n':
        database_name = optarg;
        break;
      case 'p':
        db_password = optarg;
        break;
      case 'u':
        db_username = optarg;
        break;
      case 'l': {
        cdcf::CDCFConfig config;
        config.log_level_ = optarg;
        cdcf::Logger::Init(config);
        break;
      }
      case '?':
        printf("Unknown option:  %c\n", static_cast<char>(optopt));
        exit(1);
    }
  }

  DatabaseWriteInterface* influxdb = new TradePersistInfluxdb(
      database_name, db_ip, db_port, db_username, db_password);

  TradePersistenceClient tradePersistenceClient(influxdb,
                                                order_manager_address);

  tradePersistenceClient.PersistTrades();
}
