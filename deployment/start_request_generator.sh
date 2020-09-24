#!/usr/bin/env bash

cd bin
/bin/request_generator_main -f test_env_cte_request_generator_config.json &
/bin/request_generator_main -f test_env_akka_request_generator_config.json
sleep 10m
/bin/data_verifier
sleep infinity