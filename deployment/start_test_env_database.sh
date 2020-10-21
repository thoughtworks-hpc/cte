#!/usr/bin/env bash
echo "max-values-per-tag = 0" >> /etc/influxdb/influxdb.conf
/usr/bin/influxd &
sleep infinity