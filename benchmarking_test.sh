#!/bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT=8080
NUM_CLIENTS=5
CONCURRENCY="10"
REQUESTS="100"

./server &

ab -n $REQUESTS -c $CONCURRENCY http://$SERVER_IP:$SERVER_PORT/

killall -9 server
