/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_DATA_SOURCE_H
#define CTE_DATA_SOURCE_H

#include <string>

class DataSource {
 public:
  virtual void GetDataEntries(int limit, int offset, std::string& data) = 0;
};

#endif  // CTE_DATA_SOURCE_H
