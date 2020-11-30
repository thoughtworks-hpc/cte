/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "include/trade_entity.h"

TradeEntity::TradeEntity(const match_engine_proto::Trade& trade) {
  buy_user_id_ = std::to_string(trade.buyer_user_id());
  sell_user_id_ = std::to_string(trade.seller_user_id());
  symbol_id_ = std::to_string(trade.symbol_id());
  trade_id_ = std::to_string(trade.symbol_id());
  price_ = std::to_string(trade.price());
  amount_ = std::to_string(trade.amount());
  trade_id_ = uuid::generate_uuid_v4();
  submit_time = trade.submit_time();

  if (trade.trading_side() == ::match_engine_proto::TRADING_BUY) {
    buy_order_id_ = std::to_string(trade.taker_id());
    sell_order_id_ = std::to_string(trade.maker_id());
  }

  if (trade.trading_side() == ::match_engine_proto::TRADING_SELL) {
    sell_order_id_ = std::to_string(trade.taker_id());
    buy_order_id_ = std::to_string(trade.maker_id());
  }
}

/*
 *     using time_stamp = std::chrono::time_point<std::chrono::system_clock,
        std::chrono::nanoseconds>;
    time_stamp current_time_stamp =
        std::chrono::time_point_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now());
    int64_t nanoseconds_since_epoch =
        current_time_stamp.time_since_epoch().count();

    payload.meas(config.database_table_name)
        .field("buy_order_id", trade.buy_order_id_)
        .field("sell_order_id", trade.sell_order_id_)
        .field("symbol_id", trade.symbol_id_)
        .field("trade_id", trade.trade_id_)
        .field("price", trade.price_)
        .field("amount", trade.amount_)
        .field("sell_user_id", trade.sell_user_id_)
        .field("buy_user_id", trade.buy_user_id_)
        .field("submit_time", static_cast<int64_t>(trade.submit_time))
        .timestamp(nanoseconds_since_epoch);
 *
 */

/*
 *     std::vector<database_interface::data_pair> tag;
    std::vector<database_interface::data_pair> field;
    field.emplace_back(database_interface::data_pair{
        "user_id", std::to_string(order.GetUserId())});
    field.emplace_back(database_interface::data_pair{
        "symbol", std::to_string(order.GetSymbol())});
    field.emplace_back(database_interface::data_pair{
        "price", std::to_string(order.GetPrice())});
    field.emplace_back(database_interface::data_pair{
        "amount", std::to_string(order.GetAmount())});
    field.emplace_back(database_interface::data_pair{
        "trading_side", std::to_string(order.GetTradingSide())});

    database_interface::entity payload{"orders", tag, field, i};
    influxdb.write(payload);
 */

database_interface::entity TradeEntity::to_entity(
    std::string& database_table_name) {
  using time_stamp = std::chrono::time_point<std::chrono::system_clock,
                                             std::chrono::nanoseconds>;
  time_stamp current_time_stamp =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now());
  int64_t nanoseconds_since_epoch =
      current_time_stamp.time_since_epoch().count();

  std::vector<database_interface::data_pair> tag;
  std::vector<database_interface::data_pair> field;

  field.emplace_back(
      database_interface::data_pair{"buy_order_id", this->buy_order_id_});
  field.emplace_back(
      database_interface::data_pair{"sell_order_id", this->sell_order_id_});
  field.emplace_back(
      database_interface::data_pair{"symbol_id", this->symbol_id_});
  field.emplace_back(
      database_interface::data_pair{"trade_id", this->trade_id_});
  field.emplace_back(database_interface::data_pair{"price", this->price_});
  field.emplace_back(database_interface::data_pair{"amount", this->amount_});
  field.emplace_back(
      database_interface::data_pair{"sell_user_id", this->sell_user_id_});
  field.emplace_back(
      database_interface::data_pair{"buy_user_id", this->buy_user_id_});
  field.emplace_back(database_interface::data_pair{
      "submit_time", std::to_string(this->submit_time)});

  database_interface::entity payload{database_table_name, tag, field,
                                     nanoseconds_since_epoch};
  return payload;
}
