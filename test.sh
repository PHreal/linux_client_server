#!/bin/bash
set -e  # 有錯誤立即停止
set -x  # 顯示執行過程

NUM=10
echo "Testing $NUM concurrent clients..."

# 確認伺服器是否在執行
if ! nc -z 127.0.0.1 8888 2>/dev/null; then
    echo "Server not running!"
    exit 1
fi

# 準備輸入檔
cat > input.txt << EOF
1
2
3
0
EOF

# 啟動多個 client
for i in $(seq 1 $NUM); do
    echo "啟動 client $i"
    LD_LIBRARY_PATH=$(pwd) $(pwd)/client < input.txt > log_$i.txt 2>&1 &
    sleep 0.2
done

# 等待所有 client 結束
wait
echo "All clients have finished."

# 若要保留 log 就註解掉這行
# rm -f input.txt log_*.txt