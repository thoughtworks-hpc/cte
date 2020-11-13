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

int main(int argc, char* argv[]) {
  trade_persistence_tool::Config config(argc, argv);

  DatabaseWriteInterface* influxdb = new TradePersistInfluxdb(
      config.database_name, config.db_ip, config.db_port, config.db_username,
      config.db_password, config.database_table_name);
  TradePersistenceClient tradePersistenceClient(
      influxdb, config.order_manager_address, config.database_table_name);

  std::thread th = std::thread([&] {
    influxdb_cpp::server_info si(config.db_ip, std::stoi(config.db_port),
                                 config.database_name, config.db_username,
                                 config.db_password);
    thread_local int count_ = 0;
    while (true) {
      thread_end_flag_mutex.lock();
      if (thread_end_flag) {
        trade_manager_db_buffer_mutex.lock();

        trade_manager_db_buffer_mutex.unlock();
        return;
      }
      thread_end_flag_mutex.unlock();

      trade_manager_db_buffer_mutex.lock();
      if (trade_manager_db_buffer.size() >= 5000) {
        CDCF_LOGGER_DEBUG("Try to send buffered trades data to influxdb");
        std::string resp;
        auto payload = influxdb_cpp::builder();
        for (const auto& trade : trade_manager_db_buffer) {
          payload.meas(config.database_name)
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
                "Write #{}:  buy_order_id: {}, sell_order_id: {}, symbol_id: {}, "
                "trade_id: {}, "
                " price: {}, amount: {}, sell_user_id: {}, buy_user_id: {}",
                ++count_, trade.buy_order_id_, trade.sell_order_id_,
                trade.symbol_id_, trade.trade_id_, trade.price_, trade.amount_,
                trade.sell_user_id_, trade.buy_user_id_);
          }
        } else {
          CDCF_LOGGER_ERROR("  Write db failed, ret:{} resp:{}", ret, resp);
          return;
        }
      }
      trade_manager_db_buffer_mutex.unlock();
      sleep(30);
    }
});

  tradePersistenceClient.PersistTrades();
  thread_end_flag = true;
  th.join();
  CDCF_LOGGER_CRITICAL("Trade Manager Shut Down");
}
