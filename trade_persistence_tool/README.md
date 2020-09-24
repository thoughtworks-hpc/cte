## Usage Example

```
./trade_persistence_tool_main  -m 127.0.0.1:50051 -d 127.0.0.1:8086 -t cte_trades
```
## Key Arguments
> -d, define database ip and port

> -m, define match engine ip and port

> -t, define database table name, tool will create a table named by this argument

## Optional Arguments
> -u, define database username, default is ""

> -p, define database password, default is ""

> -l, define log level, default is "info"
> Potential log levels:
> "trace", "debug", "info", "warning", "error", "critical", "off"

> -f, define log file output path, default is "/tmp/log/trade_persistence_tool.log"

> -n, define database name, tool will create a database named by this argument, default is "trade_manager"


