#!/usr/bin/env bash
/usr/bin/influxd &
sleep 10
/bin/trade_persistence_tool_main -n cte -m cte_match_engin_result_host:51001 -d 127.0.0.1:8086 -l debug &
/bin/trade_persistence_tool_main -n akka_te -m akkagateway:50051 -d 127.0.0.1:8086 -l debug