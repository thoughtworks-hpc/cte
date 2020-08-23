/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include <cdcf/actor_monitor.h>
#include <cdcf/logger.h>

#include <iostream>

#include <caf/io/all.hpp>

caf::behavior calculator_with_error(caf::event_based_actor *self) {
  self->set_default_handler(caf::reflect_and_quit);
  return {[=](caf::add_atom, int a, int b) {
    CDCF_LOGGER_INFO("received add_atom task from remote node. input a:{} b:{}",
                     a, b);
    self->quit();
    return 0;
  }};
}

int main() {
  std::cout << "using cdcf conan package" << std::endl;
  caf::actor_system_config config_;
  caf::actor_system system_{config_};
  std::promise<std::string> promise_;

  caf::scoped_actor scoped_sender(system_);
  std::string error_message_ = "";

  caf::actor supervisor_ = system_.spawn<cdcf::ActorMonitor>(
      [&](const caf::down_msg &downMsg, const std::string &actor_description) {
        promise_.set_value(caf::to_string(downMsg.reason));
      });
  caf::actor calculator_ = system_.spawn(calculator_with_error);
  cdcf::SetMonitor(supervisor_, calculator_, "worker actor for testing");

  scoped_sender->send(calculator_, caf::add_atom::value, 3, 1);
  error_message_ = promise_.get_future().get();
  std::cout << "actor monitor error message: " << error_message_ << std::endl;
}
