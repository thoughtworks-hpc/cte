/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "src/trade_persist_influxdb.h"

#include <cdcf/logger.h>

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
                .tag("buy_trade_id", buy_trade_id)
                .tag("sell_trade_id", sell_trade_id)
                .field("symbol_id", std::to_string(trade.symbol_id()))
                .field("trade_id", uuid)
                .field("price", trade.price())
                .field("amount", trade.amount())
                .field("sell_user_id", std::to_string(trade.seller_user_id()))
                .field("buy_user_id", std::to_string(trade.buyer_user_id()))
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    CDCF_LOGGER_INFO("  Write db success");
    return true;
  } else {
    CDCF_LOGGER_ERROR("  Write db failed, ret:{} resp:{}", ret, resp);
    return false;
  }
}

int TradePersistInfluxdb::CreateDatabase() {
  std::string resp;
  int ret;
  influxdb_cpp::server_info si(ip_, std::stoi(port_), database_name_, "", "");
  influxdb_cpp::query(resp, "drop database " + database_name_, si);
  ret = influxdb_cpp::create_db(resp, database_name_, si);
  if (0 != ret) {
    CDCF_LOGGER_ERROR("Creat db failed ret:{}", ret);
  } else {
    CDCF_LOGGER_INFO("Creat {} database successfully ret:{}", database_name_,
                     ret);
  }
  return ret;
}
