#!/bin/bash

SERVER="127.0.0.1"
PORT="11008"
CLIENTS=100
TIMEOUT_SEC=60

# 存储所有客户端进程ID
PIDS=()

# 启动 100 个客户端
for i in $(seq 1 $CLIENTS); do
	./socket_client -i $SERVER -p $PORT -t 1 &
	PIDS+=($!)  # $! 获取上一个后台进程的PID
done

# 等待 5 秒
sleep $TIMEOUT_SEC

# 强制终止所有客户端
for pid in "${PIDS[@]}"; do
	kill -9 "$pid" 2>/dev/null  # SIGKILL 强制终止
done

echo "All clients killed after $TIMEOUT_SEC seconds."
