/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/database_interface.hpp"

#include <cdcf/logger.h>

#include <utility>

database_interface::InfluxDB::InfluxDB(influxdb_cpp::server_info& si,
                                       const int default_size)
    : si(si) {
  CDCF_LOGGER_DEBUG("Initialize Database vector, reserve {} default size",
                    default_size);
  buffer.reserve(default_size);
  thread_end_flag = false;
  count = 0;
  th = std::thread([&]() { this->thread_main(); });
  // th = std::thread(thread_main); // require func is static
}

database_interface::InfluxDB::~InfluxDB() {
  thread_end_flag = true;
  th.join();
}

bool database_interface::InfluxDB::write(entity& data_entity) {
  buffer_mutex.lock();
  buffer.emplace_back(data_entity);
  buffer_mutex.unlock();
  return true;
}

void database_interface::InfluxDB::thread_main() {
  CDCF_LOGGER_INFO("Chunk Database IO Thread starts");
  thread_local int last_round_vector_size = 0;
  while (true) {
    CDCF_LOGGER_DEBUG(
        "Chunk Database IO Thread scans data ( waiting time: 10s )");
    if (thread_end_flag) {
      thread_write_db();
      CDCF_LOGGER_INFO("Chunk Database IO Thread ends");
      return;
    }

    buffer_mutex.lock();
    if (buffer.size() >= 5000 || (last_round_vector_size == buffer.size() &&
                                  last_round_vector_size != 0)) {
      if (thread_write_db()) {
        CDCF_LOGGER_ERROR("Write db failed, Chunk Database IO Thread ends");
        return;
      }
    }
    last_round_vector_size = buffer.size();
    buffer_mutex.unlock();

    sleep(10);
  }
}

bool database_interface::InfluxDB::thread_write_db() {
  CDCF_LOGGER_INFO("Try to send buffered {} trades data to table",
                   buffer.size());
  std::string resp;

  influxdb_cpp::detail::ts_caller payload = influxdb_cpp::detail::ts_caller();
  for (const auto& trade_entity : buffer) {
    payload.meas(trade_entity.measurement);

    for (const auto& tag : trade_entity.tag) {
      static_cast<influxdb_cpp::detail::tag_caller&>(
          (influxdb_cpp::detail::tag_caller&)payload)
          .tag("", "");
    }

    for (const auto& field : trade_entity.field) {
      static_cast<influxdb_cpp::detail::field_caller&>(
          (influxdb_cpp::detail::field_caller&)payload)
          .field("", "");
    }
  }

  int ret = payload.post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    for (const auto& trade_entity : buffer) {
      std::stringstream log;
      for (const auto& tag : trade_entity.tag) {
        log << tag.Key << ": " << tag.Value << ", ";
      }
      for (const auto& field : trade_entity.field) {
        log << field.Key << ": " << field.Value << ", ";
      }
      CDCF_LOGGER_DEBUG("Write #{}:  {}", ++count, log.str());
    }
    buffer.clear();
    return true;
  } else {
    CDCF_LOGGER_ERROR("Write db failed, ret:{} resp:{}", ret, resp);
    return false;
  }
}
