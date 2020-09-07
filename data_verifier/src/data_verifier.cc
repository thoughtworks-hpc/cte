/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/data_verifier.h"

int DataVerifier::VerifyEquality() {
  std::string data_a;
  std::string data_b;

  data_source_a_->GetDataEntries(100, 0, data_a);
  data_source_b_->GetDataEntries(100, 0, data_b);
}
