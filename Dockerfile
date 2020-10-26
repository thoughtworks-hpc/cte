FROM gcc:8.4.0 AS builder

RUN sed -i "s@http://deb.debian.org@http://mirrors.aliyun.com@g" /etc/apt/sources.list
RUN apt-get clean \
    && apt-get update \
    && apt-get install python-pip cmake vim gdb -y
RUN pip install conan \
    && conan remote add inexorgame "https://api.bintray.com/conan/inexorgame/inexor-conan" \
    && conan remote add hpc "https://api.bintray.com/conan/grandmango/cdcf"

WORKDIR /cte

COPY conanfile.txt .
RUN conan install . -s compiler.libcxx=libstdc++11 --build missing
# copy node_keeper to dir /cte
RUN find /root/.conan/data/cdcf/1.2.2/hpc/stable/package -name node_keeper | xargs cp -t .

COPY CMakeLists.txt .
COPY common common
COPY order_generator order_generator
COPY order_manager order_manager
COPY request_generator request_generator
COPY influxdb_usa_demo influxdb_usa_demo
COPY match_engine match_engine
COPY trade_persistence_tool trade_persistence_tool
COPY data_verifier data_verifier
COPY docker docker
RUN cmake . -DCMAKE_TOOLCHAIN_FILE=conan_paths.cmake -DCMAKE_BUILD_TYPE=Release \
    && cmake --build . -j 3 \
    && ctest --output-on-failure

FROM debian
RUN apt-get clean \
    && apt-get update \
    && apt-get install influxdb vim curl python procps -y
RUN apt-get clean \
    && apt-get update \
    && apt-get install influxdb-client
COPY --from=builder /cte/bin/create_orders /bin/create_orders
COPY --from=builder /cte/bin/create_initial_prices /bin/create_initial_prices
COPY --from=builder /cte/bin/request_generator_main /bin/request_generator_main
COPY --from=builder /cte/bin/match_engine_server /bin/match_engine_server
COPY --from=builder /cte/bin/order_manager /bin/order_manager
COPY --from=builder /cte/bin/trade_persistence_tool_main /bin/trade_persistence_tool_main
COPY --from=builder /cte/bin/create_initial_prices_config.json /bin/create_initial_prices_config.json
COPY --from=builder /cte/bin/create_orders_config.json /bin/create_orders_config.json
COPY --from=builder /cte/bin/data_verifier /bin/data_verifier
#COPY --from=builder /cte/bin/request_generator_config.json /bin/request_generator_config.json
COPY --from=builder /cte/node_keeper /bin/node_keeper
COPY --from=builder /cte/bin/order_manager_record_config.json /order_manager_record_config.json

COPY docker/script.sh /bin/script.sh
COPY docker/request_generator_akka_config.json /bin/request_generator_akka_config.json
COPY docker/request_generator_cte_config.json /bin/request_generator_cte_config.json
COPY docker/data_verifier_config.json /bin/data_verifier_config.json
COPY docker/order_to_akka.sh /bin/order_to_akka.sh
COPY docker/order_to_cte.sh /bin/order_to_cte.sh
COPY docker/request_generator_script.sh /bin/request_generator_script.sh
COPY docker/trade_reciever_cte.sh /bin/trade_reciever_cte.sh

COPY deployment/test_env_akka_request_generator_config.json /bin/test_env_akka_request_generator_config.json
COPY deployment/test_env_cte_request_generator_config.json /bin/test_env_cte_request_generator_config.json
COPY deployment/start_test_env_database.sh /bin/start_test_env_database.sh
COPY deployment/start_request_generator.sh /bin/start_request_generator.sh
COPY deployment/start_cte_order_manager.sh /bin/start_cte_order_manager.sh
COPY deployment/start_akka_order_manager.sh /bin/start_akka_order_manager.sh
COPY deployment/start_trade_manager.sh /bin/start_trade_manager.sh
COPY deployment/test_env_create_orders_config.json /bin/test_env_create_orders_config.json

COPY --from=builder /cte/bin/data_verifier /tmp/data_verifier
COPY deployment/test_env_data_verifier_config.json /tmp/data_verifier_config.json

CMD ["/bin/script.sh"]