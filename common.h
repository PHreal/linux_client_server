#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>

// 編譯時除錯控制
#ifdef DEBUG_COMPILE
#define COMPILE_DEBUG 1
#else
#define COMPILE_DEBUG 0
#endif

// 協定定義
#define SERVER_PORT 8888
#define BUF_SIZE 2048
#define MAX_CONN 10

// 訊息類型
#define REQ_SYSINFO 1
#define REQ_CPUINFO 2
#define REQ_MEMINFO 3

// 錯誤碼
#define OK 0
#define ERR_INVALID -1
#define ERR_TIMEOUT -2
#define ERR_CONN -3

// 訊息結構
typedef struct {
    int type;
    int len;
    int client_id;  // 客戶端 ID
    char buf[BUF_SIZE];
} Packet;

// 共用函式
void write_log(int debug_flag, const char *level, const char *msg);
int send_packet(int fd, Packet *pkt);
int recv_packet(int fd, Packet *pkt);
int check_packet(Packet *pkt);

extern int runtime_debug;

// 在 common.h 結尾加上
void fetch_sysinfo(char *buf, size_t size);
void fetch_cpuinfo(char *buf, size_t size);
void fetch_meminfo(char *buf, size_t size);

#endif