/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_DATA_SOURCE_H
#define CTE_DATA_SOURCE_H

#include <string>

class DataSource {
 public:
  virtual int GetDataEntryNumber() = 0;
  virtual void GetDataEntries(int limit, int offset, std::string& data) = 0;
  virtual std::function<bool(const std::string& source,
                             const std::string& target)>
  GetCompareFunction() = 0;
};

#endif  // CTE_DATA_SOURCE_H
