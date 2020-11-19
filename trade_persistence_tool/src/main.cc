/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <unistd.h>

#include "../../common/include/influxdb.hpp"
#include "include/config.h"
#include "include/trade_persist_influxdb.h"
#include "include/trade_persistence_client.h"

static bool thread_end_flag;
static std::mutex thread_end_flag_mutex;

bool write_data_to_db(int& count_, trade_persistence_tool::Config& config,
                      influxdb_cpp::server_info& si);

int main(int argc, char* argv[]) {
  trade_persistence_tool::Config config(argc, argv);

  DatabaseWriteInterface* influxdb = new TradePersistInfluxdb(
      config.database_name, config.db_ip, config.db_port, config.db_username,
      config.db_password, config.database_table_name);
  TradePersistenceClient tradePersistenceClient(
      influxdb, config.order_manager_address, config.database_table_name);

  std::thread th = std::thread([&] {
    CDCF_LOGGER_INFO("Chunk Database IO Thread starts");
    influxdb_cpp::server_info si(config.db_ip, std::stoi(config.db_port),
                                 config.database_name, config.db_username,
                                 config.db_password);
    thread_local int count_ = 0;
    thread_local int last_round_vector_size = 0;
    while (true) {
      CDCF_LOGGER_DEBUG(
          "Chunk Database IO Thread scans data ( waiting time: 10s )");
      thread_end_flag_mutex.lock();
      if (thread_end_flag) {
        write_data_to_db(count_, config, si);
        CDCF_LOGGER_INFO("Chunk Database IO Thread ends");
        return;
      }
      thread_end_flag_mutex.unlock();

      if (trade_manager_db_buffer.size() >= 5000 ||
          (last_round_vector_size == trade_manager_db_buffer.size() &&
           last_round_vector_size != 0)) {
        if (!write_data_to_db(count_, config, si)) {
          CDCF_LOGGER_ERROR("Write db failed, Chunk Database IO Thread ends");
          return;
        }
      }
      last_round_vector_size = trade_manager_db_buffer.size();
      sleep(10);
    }
  });

  tradePersistenceClient.PersistTrades();
  thread_end_flag = true;
  th.join();
  CDCF_LOGGER_CRITICAL("Trade Manager Shut Down");
}

bool write_data_to_db(int& count_, trade_persistence_tool::Config& config,
                      influxdb_cpp::server_info& si) {
  trade_manager_db_buffer_mutex.lock();
  CDCF_LOGGER_INFO("Try to send buffered {} trades data to {} table",
                   trade_manager_db_buffer.size(), config.database_table_name);
  std::string resp;
  auto payload = influxdb_cpp::detail::field_caller();
  for (const auto& trade : trade_manager_db_buffer) {
    payload.meas(config.database_table_name)
        .tag("buy_order_id", trade.buy_order_id_)
        .tag("sell_order_id", trade.sell_order_id_)
        .field("symbol_id", trade.symbol_id_)
        .field("trade_id", trade.trade_id_)
        .field("price", trade.price_)
        .field("amount", trade.amount_)
        .field("sell_user_id", trade.sell_user_id_)
        .field("buy_user_id", trade.buy_user_id_)
        .field("submit_time", static_cast<int64_t>(trade.submit_time));
  }
  int ret = payload.chunk_post(si, &resp);

  if (0 == ret && resp.empty()) {
    for (const auto& trade : trade_manager_db_buffer) {
      CDCF_LOGGER_DEBUG(
          "Write #{}:  buy_order_id: {}, sell_order_id: {}, symbol_id: "
          "{}, "
          "trade_id: {}, "
          " price: {}, amount: {}, sell_user_id: {}, buy_user_id: {}",
          ++count_, trade.buy_order_id_, trade.sell_order_id_, trade.symbol_id_,
          trade.trade_id_, trade.price_, trade.amount_, trade.sell_user_id_,
          trade.buy_user_id_);
    }
    trade_manager_db_buffer.clear();
    trade_manager_db_buffer_mutex.unlock();
    return true;
  } else {
    CDCF_LOGGER_ERROR("Write db failed, ret:{} resp:{}", ret, resp);
    trade_manager_db_buffer_mutex.unlock();
    return false;
  }
}
