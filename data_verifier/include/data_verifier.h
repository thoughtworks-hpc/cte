/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef DATA_VERIFIER_INCLUDE_DATA_VERIFIER_H_
#define DATA_VERIFIER_INCLUDE_DATA_VERIFIER_H_

#include <memory>

#include "./data_source.h"

class DataVerifier {
 public:
  DataVerifier(std::shared_ptr<DataSource> data_source_a,
               std::shared_ptr<DataSource> data_source_b)
      : data_source_a_(data_source_a), data_source_b_(data_source_b) {}

  bool VerifyEquality();

 private:
  std::shared_ptr<DataSource> data_source_a_;
  std::shared_ptr<DataSource> data_source_b_;
};

#endif  // DATA_VERIFIER_INCLUDE_DATA_VERIFIER_H_
