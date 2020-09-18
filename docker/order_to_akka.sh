#!/usr/bin/env bash
/usr/bin/influxd &
sleep 5
/bin/order_manager -a 0.0.0.0:50051 -d localhost:8086 -m akkagateway:50051 -r akkanode:50051 -t 1