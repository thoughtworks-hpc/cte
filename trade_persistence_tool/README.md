## Usage Example

```
./trade_persistence_tool_main -n cte -m 127.0.0.1:50051 -d 127.0.0.1:8086
```

> -n, define database name, tool will create a database named by this argument
>     and create a new table called trades 

> -b, define database ip and port

> -m, define match engine ip and port

## Optional Arguments
> -u, define database username, default is ""

> -p, define database password, default is ""

> -l, define log level, default is info
> Potential log levels:
> "trace", "debug", "info", "warning", "error", "critical", "off"

> -f, define log file output path, default is "/tmp/trade_persistence_tool.log"
