//
// Created by Yuecheng Pei on 2020/9/3.
//

#include <iostream>
#include <string>

#include "server.h"

int main(int argc, char* argv[]) {
  std::string port = "50051";
  if (argc > 1 && strcmp(argv[1], "port") == 0) {
    port = argv[2];
  }
  std::cout << "This is grpc test" << std::endl;
  MatchEngineImpl omatch_engine_impl(port);
  omatch_engine_impl.RunWithWait();
}