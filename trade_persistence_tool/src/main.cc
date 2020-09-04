//
// Created by Yuecheng Pei on 2020/9/3.
//

#include "trade_persist_influxdb.h"
#include "trade_persistence_client.h"

int main() {
  DatabaseWriteInterface* influxdb = new TradePersistInfluxdb("cte", "127.0.0.1", "8086");
  TradePersistenceClient tradePersistenceClient(influxdb, "127.0.0.1:50051");
  tradePersistenceClient.PersistTrades();
}