/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_SRC_DATABASE_INTERFACE_H_
#define REQUEST_GENERATOR_SRC_DATABASE_INTERFACE_H_

#include <string>

class DatabaseQueryInterface {
 public:
  virtual std::string GetOrders(std::string ip, std::string port) = 0;
};

#endif  // REQUEST_GENERATOR_SRC_DATABASE_INTERFACE_H_
