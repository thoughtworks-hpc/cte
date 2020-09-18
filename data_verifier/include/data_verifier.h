/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef DATA_VERIFIER_INCLUDE_DATA_VERIFIER_H_
#define DATA_VERIFIER_INCLUDE_DATA_VERIFIER_H_

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "./data_source.h"

class DataVerifier {
 public:
  DataVerifier(std::shared_ptr<DataSource> data_source_a,
               std::shared_ptr<DataSource> data_source_b,
               bool is_ordered_data_sources = true)
      : data_source_a_(data_source_a),
        data_source_b_(data_source_b),
        is_ordered_data_sources_(is_ordered_data_sources) {}

  bool VerifyEquality();
  void SetNumberOfEntriesToCompareEachTurn(int number) {
    number_of_entries_to_compare_each_turn_ = number;
  }
  void EnableCompareEntireDataSourceInOneTurn() {
    compare_entire_data_source_in_one_turn = true;
  }
  void DisableCompareEntireDataSourceInOneTurn() {
    compare_entire_data_source_in_one_turn = false;
  }

 private:
  bool VerifyEqualityForOrderedDataSource(int limit, int offset);
  bool VerifyEqualityForUnorderedDataSource(int limit, int offset);

  std::shared_ptr<DataSource> data_source_a_;
  std::shared_ptr<DataSource> data_source_b_;
  bool is_ordered_data_sources_ = true;
  bool compare_entire_data_source_in_one_turn = true;
  int number_of_entries_to_compare_each_turn_ = 10000;
};

#endif  // DATA_VERIFIER_INCLUDE_DATA_VERIFIER_H_
