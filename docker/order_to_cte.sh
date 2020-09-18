#!/usr/bin/env bash
/usr/bin/influxd &
sleep 5
/bin/order_manager -a 0.0.0.0:50051 -d localhost:8086 -m cte_match_engin_result_host:51001 -t $TEST_MODE_OPEN