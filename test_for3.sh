#!/bin/bash

NUM=15
echo "========================================="
echo "Testing $NUM concurrent clients with unique IDs"
echo "========================================="

# 檢查伺服器
if ! nc -z 127.0.0.1 8888 2>/dev/null; then
    echo "Server not running!"
    echo "Please start server first: make run-server-debug"
    exit 1
fi

# 建立測試輸入
cat > input.txt << EOF
1
2
3
0
EOF

echo "Starting clients..."
# 啟動客戶端，每個有唯一 ID
for i in $(seq 1 $NUM); do
    CLIENT_ID=$((1000 + i))
    echo "  Starting Client #$CLIENT_ID (instance $i)"
    LD_LIBRARY_PATH=. ./client --id $CLIENT_ID < input.txt > log_client_${CLIENT_ID}.txt 2>&1 &
    PIDS[$i]=$!
    sleep 0.2
done

echo ""
echo "Waiting for clients to complete..."
wait

# 統計
SUCCESS=0
for i in $(seq 1 $NUM); do
    CLIENT_ID=$((1000 + i))
    if grep -q "connected" log_client_${CLIENT_ID}.txt; then
        SUCCESS=$((SUCCESS + 1))
        echo "✓ Client #$CLIENT_ID succeeded"
    else
        echo "✗ Client #$CLIENT_ID failed"
    fi
done

echo ""
echo "========================================="
echo "Result: $SUCCESS/$NUM clients succeeded"
echo "========================================="

# 顯示示例日誌
echo ""
echo "Sample log from Client #1001:"
echo "-------------------------------------"
head -15 log_client_1001.txt
echo "-------------------------------------"

echo ""
echo "Log files generated:"
ls -lh log_client_*.txt

# 清理選項
echo ""
read -p "Delete log files? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -f input.txt log_client_*.txt
    echo "Logs deleted"
fi