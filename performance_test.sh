#!/bin/bash

LOG_FILE="performance_logs.txt"
SERVER_URL="http://localhost:8080"
REQUEST_COUNT=1000

./server &

echo "Testing server performance..."
echo "-----------------------------"

start_time=$(date +%s%N)

# Send requests using curl and measure the time taken for each request
for ((i=0; i < $REQUEST_COUNT; i++)); do
    start_req_time=$(date +%s%N)
    curl -sS -o /dev/null $SERVER_URL
    end_req_time=$(date +%s%N)
    duration=$((($end_req_time - $start_req_time) / 1000000))  # Convert nanoseconds to milliseconds
    echo "Request $((i+1)): $duration ms"
done

end_time=$(date +%s%N)
overall_duration=$((($end_time - $start_time) / 1000000000))  # Convert nanoseconds to seconds

echo "-----------------------------"
echo "Testing completed."
echo "Overall Duration: $overall_duration s" >> $LOG_FILE

killall -9 server
