/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include <string>

#include "./server.h"

int main(int argc, char* argv[]) {
  std::string port = "50055";
  if (argc > 1 && strcmp(argv[1], "port") == 0) {
    port = argv[2];
  }
  std::cout << "This is grpc test" << std::endl;
  OrderManagerImpl order_manager_impl(port);
  order_manager_impl.RunWithWait();
}
