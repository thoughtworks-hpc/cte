/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef DATA_VERIFIER_INCLUDE_DATA_SOURCE_H_
#define DATA_VERIFIER_INCLUDE_DATA_SOURCE_H_

#include <functional>
#include <string>

class DataSource {
 public:
  virtual int GetDataEntryNumber() = 0;
  virtual void GetDataEntries(int limit, int offset, std::string& data) = 0;
  virtual std::function<bool(const std::string& source,
                             const std::string& target)>
  GetCompareFunction() = 0;
};

#endif  // DATA_VERIFIER_INCLUDE_DATA_SOURCE_H_

