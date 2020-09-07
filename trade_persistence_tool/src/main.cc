/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <unistd.h>

#include "./trade_persist_influxdb.h"
#include "./trade_persistence_client.h"

int main(int argc, char* argv[]) {
  std::string db_ip;
  std::string db_port;
  std::string database_name;
  std::string order_manager_address;

  int option;
  while ((option = getopt(argc, argv, "m:d:n:")) != -1) {
    switch (option) {
      case 'm':
        order_manager_address = optarg;
        break;
      case 'd':
        db_ip = ParseIpAddress(optarg)[0];
        db_port = ParseIpAddress(optarg)[1];
        std::cout << db_ip << ", " << db_port << std::endl;
        break;
      case 'n':
        database_name = optarg;
        break;
      case '?':
        printf("Unknown option:  %c\n", static_cast<char>(optopt));
        exit(1);
    }
  }

  DatabaseWriteInterface* influxdb =
      new TradePersistInfluxdb(database_name, db_ip, db_port);
  TradePersistenceClient tradePersistenceClient(influxdb,
                                                order_manager_address);

  tradePersistenceClient.PersistTrades();
}
