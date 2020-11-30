/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/database_interface.hpp"

database_interface::InfluxDB::InfluxDB(influxdb_cpp::server_info& si,
                                       const bool enable_thread,
                                       const int default_size)
    : si(si), enable_thread(enable_thread) {
  CDCF_LOGGER_DEBUG("Initialize Database vector, reserve {} default size",
                    default_size);
  buffer.reserve(default_size);

  count = 0;

  if (enable_thread) {
    thread_end_flag = false;
    th = std::thread([&]() { this->thread_main(); });
    // th = std::thread(thread_main); // require func is static
  }
}

database_interface::InfluxDB::~InfluxDB() {
  if (enable_thread) {
    CDCF_LOGGER_INFO("deconstruct of InfluxDB has been called");
    thread_end_flag = true;
    th.join();
  }
}

bool database_interface::InfluxDB::write(entity& data_entity) {
  if (enable_thread) {
    buffer_mutex.lock();

    buffer.emplace_back(data_entity);

    if (buffer.size() >= 10000) {
      flush_buffer();
    }
    buffer_mutex.unlock();
  } else {
    // buffer.emplace_back(data_entity);
    buffer.push_back(std::move(data_entity));

    if (buffer.size() >= 10000) {
      flush_buffer();
    }
  }

  return true;
}

void database_interface::InfluxDB::thread_main() {
  CDCF_LOGGER_INFO("Chunk Database IO Thread starts");
  thread_local int last_round_vector_size = 0;
  while (true) {
    CDCF_LOGGER_DEBUG(
        "Chunk Database IO Thread scans data ( waiting time: 5s )");
    buffer_mutex.lock();
    bool res = flush_buffer();
    buffer_mutex.unlock();
    if (!res) {
      CDCF_LOGGER_ERROR("Write db failed, Chunk Database IO Thread ends");
      return;
    }

    if (thread_end_flag && 0 == buffer.size()) {
      CDCF_LOGGER_INFO("Chunk Database IO Thread ends");
      return;
    }
    sleep(5);
  }
}

bool database_interface::InfluxDB::flush_buffer() {
  std::string resp;

  influxdb_cpp::detail::field_caller payload =
      influxdb_cpp::detail::field_caller();

  for (int i = 0; i < buffer.size(); ++i) {
    if (i == 0) {
      CDCF_LOGGER_INFO("Try to send buffered {} trades to {}", buffer.size(),
                       buffer[0].measurement);
    }

    const auto& trade_entity = buffer[i];
    payload.meas(trade_entity.measurement);
    for (const auto& tag : trade_entity.tag) {
      static_cast<influxdb_cpp::detail::tag_caller&>(
          (influxdb_cpp::detail::tag_caller&)payload)
          .tag(tag.Key, tag.Value);
    }

    for (int j = 0; j < trade_entity.field.size(); ++j) {
      auto field = trade_entity.field[j];
      if (0 == j) {
        static_cast<influxdb_cpp::detail::tag_caller&>(
            (influxdb_cpp::detail::tag_caller&)payload)
            .field(field.Key, field.Value);
      } else {
        static_cast<influxdb_cpp::detail::field_caller&>(
            (influxdb_cpp::detail::field_caller&)payload)
            .field(field.Key, field.Value);
      }
    }
    payload.timestamp(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    // CDCF_LOGGER_INFO("{}", payload.get_line());
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
      // CDCF_LOGGER_DEBUG("Write #{}:  {}", ++count, log.str());
    }
    buffer.clear();
    return true;
  } else {
    CDCF_LOGGER_ERROR("Write db failed, ret:{} resp:{}", ret, resp);
    return false;
  }
}

database_interface::entity::entity(const std::string& measurement,
                                   const std::vector<data_pair>& tag,
                                   const std::vector<data_pair>& field,
                                   int64_t timestamp)
    : measurement(measurement), tag(tag), field(field), timestamp(timestamp) {}
