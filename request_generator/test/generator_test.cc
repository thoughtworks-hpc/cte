/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <gmock/gmock.h>
#include <grpcpp/create_channel.h>

#include "src/config.h"

using testing::Eq;

TEST(ConfigTest, check_config_initialization) {
  request_generator::Config config("request_generator_config_test.json");
  EXPECT_THAT(config.default_num_of_requests_, Eq(1));
  EXPECT_THAT(config.default_num_of_threads_, Eq(1));
  EXPECT_THAT(config.default_db_host_address_, Eq("127.0.0.1"));
  EXPECT_THAT(config.default_db_port_, Eq("8086"));
}
