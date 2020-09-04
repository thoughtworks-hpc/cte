#!/usr/bin/env bash
/bin influxd &
/bin order_manager -a localhost:50051 -d localhost:8086 -m akkagateway:50051 -r akkanode:50051