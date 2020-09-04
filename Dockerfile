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
RUN find /root/.conan/data/cdcf/1.0/hpc/stable/package -name node_keeper | xargs cp -t .

COPY CMakeLists.txt .
COPY common common
COPY order_generator order_generator
COPY order_manager order_manager
COPY request_generator request_generator
COPY influxdb_usa_demo influxdb_usa_demo
COPY match_engine match_engine
COPY docker docker
RUN cmake . -DCMAKE_TOOLCHAIN_FILE=conan_paths.cmake -DCMAKE_BUILD_TYPE=Release \
    && cmake --build . -j 3 \
    && ctest --output-on-failure

FROM debian
RUN apt-get clean \
    && apt-get update \
    && apt-get install influxdb -y
RUN apt-get clean \
    && apt-get update \
    && apt-get install influxdb-client
COPY --from=builder /cte/bin/create_orders /bin/create_orders
COPY --from=builder /cte/bin/create_initial_prices /bin/create_initial_prices
COPY --from=builder /cte/bin/request_generator_main /bin/request_generator_main
COPY --from=builder /cte/bin/match_engine_server /bin/match_engine_server
COPY --from=builder /cte/bin/order_manager /bin/order_manager
COPY --from=builder /cte/bin/create_initial_prices_config.json /bin/create_initial_prices_config.json
COPY --from=builder /cte/bin/create_orders_config.json /bin/create_orders_config.json
COPY --from=builder /cte/bin/request_generator_config.json /bin/request_generator_config.json
COPY --from=builder /cte/node_keeper /bin/node_keeper

COPY docker/script.sh /bin/script.sh

CMD ["/bin/script.sh"]