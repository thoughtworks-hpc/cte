/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "src/trade_persist_influxdb.h"

#include <string>

#include "../../common/include/influxdb.hpp"

bool TradePersistInfluxdb::PersistTrade(const match_engine_proto::Trade& trade,
                                        std::string uuid) {
  influxdb_cpp::server_info si(ip_, std::stoi(port_), database_name_, "", "");
  std::string resp;

  std::string buy_trade_id;
  std::string sell_trade_id;
  if (trade.trading_side() == ::match_engine_proto::TRADING_BUY) {
    buy_trade_id = std::to_string(trade.taker_id());
    sell_trade_id = std::to_string(trade.maker_id());
  }

  if (trade.trading_side() == ::match_engine_proto::TRADING_SELL) {
    sell_trade_id = std::to_string(trade.taker_id());
    buy_trade_id = std::to_string(trade.maker_id());
  }

  int ret = influxdb_cpp::builder()
                .meas("trades")
                .tag("symbol_id", std::to_string(trade.symbol_id()))
                .tag("trade_id", uuid)
                .field("buy_trade_id", buy_trade_id)
                .field("sell_trade_id", sell_trade_id)
                .field("price", trade.price())
                .field("amount", trade.amount())
                .field("seller_user_id", std::to_string(trade.seller_user_id()))
                .field("buyer_user_id", std::to_string(trade.buyer_user_id()))
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    std::cout << "write db success" << std::endl;
    return true;
  } else {
    std::cout << "write db failed, ret:" << ret << " resp:" << resp
              << std::endl;
    return false;
  }
}

int TradePersistInfluxdb::CreateDatabase() {
  std::string resp;
  int ret;
  influxdb_cpp::server_info si(ip_, std::stoi(port_), database_name_, "", "");
  ret = influxdb_cpp::create_db(resp, database_name_, si);
  if (0 != ret) {
    std::cout << "creat db failed ret:" << ret << std::endl;
  }
  return ret;
}
