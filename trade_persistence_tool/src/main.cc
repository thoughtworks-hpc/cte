/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <cdcf/logger.h>
#include <unistd.h>

#include "include/config.h"
#include "include/trade_persist_influxdb.h"
#include "include/trade_persistence_client.h"

int main(int argc, char* argv[]) {
  trade_persistence_tool::Config config(argc, argv);

  DatabaseWriteInterface* influxdb = new TradePersistInfluxdb(
      config.database_name, config.db_ip, config.db_port, config.db_username,
      config.db_password, config.database_table_name);
  TradePersistenceClient tradePersistenceClient(
      influxdb, config.order_manager_address, config.database_table_name);

  tradePersistenceClient.PersistTrades();
  std::thread t([&tradePersistenceClient, &config]() {
    while (true) {
      std::this_thread::sleep_for(std::chrono::minutes(10));
      CDCF_LOGGER_INFO("receive trade count for {}: {}",
                       config.database_table_name,
                       tradePersistenceClient.GetReceivedTradeCount());
    }
  });
  t.join();
}
