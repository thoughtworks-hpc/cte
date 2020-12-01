/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <unistd.h>

#include "../../common/include/database_interface.hpp"
#include "../../common/include/influxdb.hpp"
#include "include/config.h"
#include "include/trade_persist_influxdb.h"
#include "include/trade_persistence_client.h"

int main(int argc, char* argv[]) {
  trade_persistence_tool::Config config(argc, argv);

  influxdb_cpp::server_info si(config.db_ip, std::stoi(config.db_port),
                               config.database_name, config.db_username,
                               config.db_password);
  database_interface::InfluxDB influxdb(si);

  TradePersistenceClient tradePersistenceClient(
      &influxdb, config.order_manager_address, config.database_table_name);

  tradePersistenceClient.PersistTrades();

  CDCF_LOGGER_CRITICAL("Trade Manager Shut Down");
}
