#!/usr/bin/env bash
/bin influxd &
/bin order_manager -a localhost:50051 -d localhost:8086 -m cte_match_engin_result_host:51001