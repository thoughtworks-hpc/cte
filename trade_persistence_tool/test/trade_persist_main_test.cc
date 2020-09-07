/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <gmock/gmock.h>

#include "src/trade_persistence_client.h"

using testing::Eq;

TEST(ParseIpAddress, should_parse_address_correctly_when_given_legal_input) {
  std::string address = "192.168.0.1:50051";
  std::string ip = ParseIpAddress(address)[0];
  std::string port = ParseIpAddress(address)[1];
  EXPECT_THAT(ip, Eq("192.168.0.1"));
  EXPECT_THAT(port, Eq("50051"));
}
