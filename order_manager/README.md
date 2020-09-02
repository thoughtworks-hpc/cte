# Usage Example
> -a, --service_address: Specify order manager service address;

> -d, --database_address: Specify database address which is used by the order manager

> -m, --match_engine_main_address: Specify match engine main address

> -r, --match_engine_request_addresses: Specify match engine request only addresses

> -h, --help: Print Usage

```shell script
./order_manager -a localhost:50051 -d localhost:8086 -m localhost:50052 -r localhost:50053,localhost:50054
```