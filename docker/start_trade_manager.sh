#!/usr/bin/env bash

sleep 5
/bin/trade_persistence_tool_main -t cte_trades -m 172.30.28.10:51001 -d 172.30.28.18:8086 -l debug -f /tmp/cte_trade_persistence_tool.log
#/bin/trade_persistence_tool_main -t akka_te_trades -m akkagateway:50051 -d 127.0.0.1:8086 -l debug -f /tmp/akka_te_trade_persistence_tool.log