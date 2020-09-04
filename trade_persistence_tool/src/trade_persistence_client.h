//
// Created by Yuecheng Pei on 2020/9/3.
//

#ifndef CTE_TRADE_PERSISTENCE_CLIENT_H
#define CTE_TRADE_PERSISTENCE_CLIENT_H

#include <string>
#include <utility>

#include "database_write_interface.h"

class TradePersistenceClient {
 public:
  TradePersistenceClient(DatabaseWriteInterface* database,
                         const std::string& tradeEngineAddress)
      : database(database), trade_engine_address_(tradeEngineAddress) {}

  bool PersistTrades();

 private:
  DatabaseWriteInterface* database;
  std::string trade_engine_address_;
};

#endif  // CTE_TRADE_PERSISTENCE_CLIENT_H
