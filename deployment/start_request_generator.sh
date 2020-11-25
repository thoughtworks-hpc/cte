#!/usr/bin/env bash

i=0
while true; do
  #  curl -POST http://172.30.28.8:8086/query -s --data-urlencode "q=DROP DATABASE order_manager"
  #  curl -POST http://172.30.28.8:8086/query -s --data-urlencode "q=DROP DATABASE trade_manager"

  echo "[IMPORTANT] round $i starts"
  echo "[IMPORTANT] round $i starts" >>/tmp/log/long_run_status.log

  if [[ $i -eq 0 ]]; then
    curl -POST http://172.30.28.30:8086/query -s --data-urlencode "q=DROP DATABASE orders"
    curl -POST http://172.30.28.30:8086/query -s --data-urlencode "q=DROP DATABASE akka_order_manager"
    curl -POST http://172.30.28.30:8086/query -s --data-urlencode "q=DROP DATABASE cte_order_manager"
    curl -POST http://172.30.28.30:8086/query -s --data-urlencode "q=DROP DATABASE trade_manager"

    curl -POST http://172.30.28.30:8086/query -s --data-urlencode "q=CREATE DATABASE akka_order_manager"
    curl -POST http://172.30.28.30:8086/query -s --data-urlencode "q=CREATE DATABASE cte_order_manager"
    curl -POST http://172.30.28.30:8086/query -s --data-urlencode "q=CREATE DATABASE trade_manager"
  fi

  cd /bin
  /bin/create_initial_prices
  echo '[IMPORTANT] initial prices are generated'
  echo '[IMPORTANT] initial prices are generated' >>/tmp/log/long_run_status.log
  /bin/create_orders initial_prices.json test_env_create_orders_config.json
  echo '[IMPORTANT] initial orders are generated'
  echo '[IMPORTANT] initial orders are generated' >>/tmp/log/long_run_status.log

  cte_trades_count=$(curl -GET 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode "db=trade_manager" --data-urlencode "q=SELECT count(symbol_id) FROM cte_trades" | python -c 'import json,sys;obj=json.load(sys.stdin); print(obj["results"][0]["series"][0]["values"][0][1])')
  akka_te_trades_count=$(curl -GET 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode "db=trade_manager" --data-urlencode "q=SELECT count(symbol_id) FROM akka_te_trades" | python -c 'import json,sys;obj=json.load(sys.stdin); print(obj["results"][0]["series"][0]["values"][0][1])')

  echo "[IMPORTANT] cte_trades_count: ${cte_trades_count}"
  echo "[IMPORTANT] cte_trades_count: ${cte_trades_count}" >>/tmp/log/long_run_status.log
  echo "[IMPORTANT] akka_te_trades_count: ${akka_te_trades_count}"
  echo "[IMPORTANT] akka_te_trades_count: ${akka_te_trades_count}" >>/tmp/log/long_run_status.log

  echo "[IMPORTANT] start generating $NUM_OF_REQUEST requests to cte and akka-te"
  echo "[IMPORTANT] start generating $NUM_OF_REQUEST requests to cte and akka-te" >>/tmp/log/long_run_status.log

  /bin/request_generator_main -n $NUM_OF_REQUEST -f test_env_cte_request_generator_config.json &
  cte_pid=$!
  /bin/request_generator_main -n $NUM_OF_REQUEST -f test_env_akka_request_generator_config.json
  echo '[IMPORTANT] akka-te is finished, waiting for database service'
  echo '[IMPORTANT] akka-te is finished, waiting for database service' >>/tmp/log/long_run_status.log

  while true; do
    PID_EXIST=$(ps aux | awk '{print $2}' | grep -w $cte_pid)
    if [ ! $PID_EXIST ]; then
      echo '[IMPORTANT] cte is finished, waiting for database service'
      echo '[IMPORTANT] cte is finished, waiting for database service' >>/tmp/log/long_run_status.log
      break
    fi
    echo 'cte is still running, try sleep 30 seconds...'
    echo 'cte is still running, try sleep 30 seconds...' >>/tmp/log/long_run_status.log
    sleep 30
  done

  while true; do
    count1=$(curl -GET 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode "db=trade_manager" --data-urlencode "q=SELECT count(symbol_id) FROM cte_trades" | python -c 'import json,sys;obj=json.load(sys.stdin); print(obj["results"][0]["series"][0]["values"][0][1])')
    sleep 15
    count2=$(curl -GET 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode "db=trade_manager" --data-urlencode "q=SELECT count(symbol_id) FROM cte_trades" | python -c 'import json,sys;obj=json.load(sys.stdin); print(obj["results"][0]["series"][0]["values"][0][1])')
    if [ $count1 == $count2 ]; then
      echo '[IMPORTANT] cte database is available now'
      echo '[IMPORTANT] cte database is available now' >>/tmp/log/long_run_status.log
      break
    fi
    echo 'cte database is still busy, try to sleep 15 seconds...'
    echo 'cte database is still busy, try to sleep 15 seconds...' >>/tmp/log/long_run_status.log
  done

  while true; do
    count1=$(curl -GET 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode "db=trade_manager" --data-urlencode "q=SELECT count(symbol_id) FROM akka_te_trades" | python -c 'import json,sys;obj=json.load(sys.stdin); print(obj["results"][0]["series"][0]["values"][0][1])')
    sleep 15
    count2=$(curl -GET 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode "db=trade_manager" --data-urlencode "q=SELECT count(symbol_id) FROM akka_te_trades" | python -c 'import json,sys;obj=json.load(sys.stdin); print(obj["results"][0]["series"][0]["values"][0][1])')
    if [ $count1 == $count2 ]; then
      echo '[IMPORTANT] akka_te database is available now'
      echo '[IMPORTANT] akka_te database is available now' >>/tmp/log/long_run_status.log
      break
    fi
    echo 'akka_te database is still busy, try to sleep 15 seconds...'
    echo 'akka_te database is still busy, try to sleep 15 seconds...' >>/tmp/log/long_run_status.log
  done

  echo '[IMPORTANT] start data_verifier now'
  echo '[IMPORTANT] start data_verifier now' >>/tmp/log/long_run_status.log
  cd /tmp
  /tmp/data_verifier
  result=$?
  check_result=0
  if [[ $result -ne $check_result ]]; then
    break
  fi

  i=$((i + 1))

  curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=orders' -s --data-urlencode "q=select * into orders_backup_${i} from orders"
  curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=orders' -s --data-urlencode "q=drop measurement orders"

  curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=cte_order_manager' -s --data-urlencode 'q=select * into order_backup_'${i}' from "order"'
  curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=cte_order_manager' -s --data-urlencode 'q=drop measurement "order"'
  curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=akka_order_manager' -s --data-urlencode 'q=select * into order_backup_'${i}' from "order"'
  curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=akka_order_manager' -s --data-urlencode 'q=drop measurement "order"'

  curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=trade_manager' -s --data-urlencode "q=select * into akka_te_trades_backup_${i} from akka_te_trades"

  while true; do
    echo '[IMPORTANT] start to clear akka_te_trades from trade_manager database'
    echo '[IMPORTANT] start to clear akka_te_trades from trade_manager database' >>/tmp/log/long_run_status.log
    curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=trade_manager' -s --data-urlencode "q=drop measurement akka_te_trades"
    count=$(curl -GET 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode "db=trade_manager" --data-urlencode "q=SELECT count(symbol_id) FROM akka_te_trades" | python -c 'import json,sys;obj=json.load(sys.stdin); print(obj["results"][0]["series"][0]["values"][0][1])')
    if [ -z "$count" ]; then
      echo '[IMPORTANT] akka_te_trades cleared'
      echo '[IMPORTANT] akka_te_trades cleared' >>/tmp/log/long_run_status.log
      break
    fi
  done

  curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=trade_manager' -s --data-urlencode "q=select * into cte_trades_backup_${i} from cte_trades"

  while true; do
    echo '[IMPORTANT] start to clear cte_trades from trade_manager database'
    echo '[IMPORTANT] start to clear cte_trades from trade_manager database' >>/tmp/log/long_run_status.log
    curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=trade_manager' -s --data-urlencode "q=drop measurement cte_trades"
    count=$(curl -GET 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode "db=trade_manager" --data-urlencode "q=SELECT count(symbol_id) FROM cte_trades" | python -c 'import json,sys;obj=json.load(sys.stdin); print(obj["results"][0]["series"][0]["values"][0][1])')
    if [ -z "$count" ]; then
      echo '[IMPORTANT] cte_trades cleared'
      echo '[IMPORTANT] cte_trades cleared' >>/tmp/log/long_run_status.log
      break
    fi
  done

  temp_num1=$(($i % 5))
  temp_num2=$(($i / 5))
  if [[ $temp_num1 -eq 0 && $temp_num2 -gt 2 ]]; then
    for j in {1..5}; do
      temp_num3=$(($i - 15 + $j))
      echo $i:$temp_num3
      echo $i:$temp_num3 >>/tmp/log/long_run_status.log
      curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=orders' -s --data-urlencode "q=drop measurement orders_backup_${temp_num3}"
      curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=cte_order_manager' -s --data-urlencode "q=drop measurement order_backup_${temp_num3}"
      curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=akka_order_manager' -s --data-urlencode "q=drop measurement order_backup_${temp_num3}"
      curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=trade_manager' -s --data-urlencode "q=drop measurement akka_te_trades_backup_${temp_num3}"
      curl -POST 'http://172.30.28.30:8086/query?pretty=true' -s --data-urlencode 'db=trade_manager' -s --data-urlencode "q=drop measurement cte_trades_backup_${temp_num3}"
    done
  fi
done
sleep infinity
