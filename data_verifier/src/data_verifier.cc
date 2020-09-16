/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */
#include "../include/data_verifier.h"

#include <cdcf/logger.h>

#include <iostream>
#include <thread>

bool DataVerifier::VerifyEquality() {
  std::string data_a;
  std::string data_b;

  int data_source_a_number = data_source_a_->GetDataEntryNumber();
  int data_source_b_number = data_source_b_->GetDataEntryNumber();

  if (data_source_a_number != data_source_b_number) {
    CDCF_LOGGER_INFO("trade data entry number inconsistent between {} and {}",
                     data_source_a_number, data_source_b_number);
    return false;
  }
  int data_source_entry_number = data_source_a_number;

  //  int data_source_number_remaining = data_source_a_number;
  //  int data_source_number_to_retrieve = 10000;
  //  int offset = 0;

  int compare_turn_count =
      std::ceil(static_cast<float>(data_source_a_number) /
                static_cast<float>(number_of_entries_to_compare_each_turn_));

  int thread_number = compare_turn_count > std::thread::hardware_concurrency()
                          ? std::thread::hardware_concurrency()
                          : compare_turn_count;
  thread_number = 1;

  int entry_number_each_thread = data_source_entry_number / thread_number;

  CDCF_LOGGER_INFO("start using {} threads", thread_number);

  if (thread_number == 1) {
    VerifyEquality(data_source_entry_number, 0);
  } else {
    int entry_number = entry_number_each_thread;
    int offset = 0;
    for (int i = 1; i <= thread_number; ++i) {
      if (i == thread_number) {
        entry_number = data_source_entry_number -
                       (entry_number_each_thread * (thread_number - 1));
      }
      threads_.emplace_back([this, entry_number, offset]() {
        VerifyEquality(entry_number, offset);
      });
      offset += entry_number;
    }
  }

  for (auto& thread : threads_) {
    thread.join();
  }

  //  while (data_source_number_remaining > 0) {
  //    auto data_entries =
  //        data_source_a_->GetDataEntries(data_source_number_to_retrieve,
  //        offset);
  //
  //    for (const auto& data_entry : data_entries) {
  //      if (!data_source_b_->FindIfDataEntryExists(data_entry)) {
  //        CDCF_LOGGER_ERROR("cannot find corresponding data entry {}",
  //                          data_entry);
  //        return false;
  //      }
  //    }
  //
  //    data_source_number_remaining -= data_source_number_to_retrieve;
  //    offset += data_source_number_to_retrieve;
  //    CDCF_LOGGER_INFO("{} entries compared with {} remaining",
  //                     data_source_number_to_retrieve,
  //                     data_source_number_remaining);
  //  }
  CDCF_LOGGER_INFO("finish");
  CDCF_LOGGER_INFO("finish");

  return !inconsistency_found_.load();
}

void DataVerifier::VerifyEquality(int limit, int offset) {
  int data_source_number_remaining = limit;
  int data_source_number_to_retrieve = number_of_entries_to_compare_each_turn_;

  CDCF_LOGGER_INFO("VerifyEquality with limit {} & offset {}", limit, offset);

  while (data_source_number_remaining > 0) {
    if (inconsistency_found_.load()) {
      return;
    }

    auto data_entries =
        data_source_a_->GetDataEntries(data_source_number_to_retrieve, offset);

    for (const auto& data_entry : data_entries) {
      if (!data_source_b_->FindIfDataEntryExists(data_entry)) {
        CDCF_LOGGER_ERROR(
            "cannot find corresponding data entry {} from second data source",
            data_entry);
        inconsistency_found_.store(true);
        return;
      }
    }

    data_source_number_remaining -= data_source_number_to_retrieve;
    offset += data_source_number_to_retrieve;
    CDCF_LOGGER_INFO("{} entries compared with {} remaining",
                     data_source_number_to_retrieve,
                     data_source_number_remaining);
  }
}
