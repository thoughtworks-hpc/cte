/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <unistd.h>

#include "./config.h"
#include "./trade_persist_influxdb.h"
#include "./trade_persistence_client.h"

int main(int argc, char* argv[]) {
  trade_persistence_tool::Config config(argc, argv);

  DatabaseWriteInterface* influxdb = new TradePersistInfluxdb(
      config.database_name, config.db_ip, config.db_port, config.db_username,
      config.db_password);
  TradePersistenceClient tradePersistenceClient(influxdb,
                                                config.order_manager_address);

  tradePersistenceClient.PersistTrades();
}
