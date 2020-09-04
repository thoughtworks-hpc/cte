/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <unistd.h>

#include "./trade_persist_influxdb.h"
#include "./trade_persistence_client.h"

std::vector<std::string> ParseIpAddress(std::string address) {
  std::vector<std::string> output;
  std::string ip;
  std::string port;
  for (int i = 0; i < address.size(); i++) {
    if (address[i] == ':') {
      port += address[i + 1];
      i = i + 2;
    }
    if (port.empty()) {
      ip += address[i];
    } else {
      port += address[i];
    }
  }
  output.push_back(ip);
  output.push_back(port);
  return output;
}

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
