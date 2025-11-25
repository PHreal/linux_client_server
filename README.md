# C 客戶端-伺服器應用程式

## 功能

1. **多程序處理**：使用 fork() 為每個客戶端建立子程序
2. **並發支援**：最多 10 個客戶端同時連線
3. **系統資訊查詢**：系統、CPU、記憶體資訊
4. **動態函式庫**：共用函式封裝在 libcommon.so
5. **除錯控制**：編譯時和執行時兩層除錯

## 編譯

```bash
# 一般編譯
make

# 編譯除錯版本
make debug

# 清理
make clean
```

## 執行

### 伺服器

```bash
# 一般模式
make run-server

# 除錯模式
make run-server-debug
```

### 客戶端

```bash
# 一般模式
make run-client

# 除錯模式
make run-client-debug

# 指定客戶端 ID
LD_LIBRARY_PATH=. ./client --id 100

# 連線到遠端並指定 ID
LD_LIBRARY_PATH=. ./client --server 192.168.0.000 --id 000
```

### 客戶端 ID 說明

- 每個客戶端都有唯一的 ID
- 預設使用程序 ID (PID % 10000)
- 可用 `--id` 參數手動指定
- 伺服器日誌會顯示每個客戶端的 ID
- 方便追蹤多個客戶端的行為

## 並發測試

```bash
chmod +x test.sh
./test.sh
```

## 錯誤處理機制

### 範例 1：訊息驗證

**無驗證**（危險）：
```c
send(fd, pkt, sizeof(Packet), 0);  // 可能崩潰
```

**有驗證**（安全）：
```c
if (check_packet(pkt) != OK) return ERR_INVALID;
send_packet(fd, pkt);  // 安全
```

### 範例 2：連線逾時

**無逾時**：可能永久阻塞
**有逾時**：30 秒後返回錯誤

### 範例 3：並發限制

**無限制**：系統資源耗盡
**有限制**：最多 10 個連線，其餘拒絕

## 封包擷取

### tcpdump

```bash
# 擷取封包
sudo tcpdump -i lo -w capture.pcap 'port 8888'

# 分析
tcpdump -r capture.pcap -A
```


## 測試步驟

```bash
# 1. 編譯
make

# 2. 啟動伺服器（終端 1）
make run-server-debug

# 3. 啟動客戶端（終端 2）
make run-client-debug

# 4. 測試所有功能

# 5. 並發測試
./test.sh

# 6. 封包擷取（終端 3）
sudo tcpdump -i lo -w test.pcap 'port 8888'
```

## 檔案說明

- `common.h` - 共用標頭
- `common.c` - 共用函式庫
- `server.c` - 伺服器
- `client.c` - 客戶端
- `Makefile` - 建置檔
- `test.sh` - 並發測試