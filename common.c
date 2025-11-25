#include "common.h"

int runtime_debug = 0;

// 寫入日誌
void write_log(int debug_flag, const char *level, const char *msg) {
    if (!COMPILE_DEBUG && !debug_flag) {
        return;
    }
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", t);
    
    fprintf(stderr, "[%s][%s] %s\n", timestamp, level, msg);
}

// 驗證封包
int check_packet(Packet *pkt) {
    if (!pkt) return ERR_INVALID;
    if (pkt->type < REQ_SYSINFO || pkt->type > REQ_MEMINFO) return ERR_INVALID;
    if (pkt->len < 0 || pkt->len > BUF_SIZE) return ERR_INVALID;
    if (pkt->client_id < 0) return ERR_INVALID;
    return OK;
}

// 發送封包
int send_packet(int fd, Packet *pkt) {
    if (fd < 0) return ERR_CONN;
    if (check_packet(pkt) != OK) return ERR_INVALID;
    
    int total = sizeof(int) * 3 + pkt->len;  // type + len + client_id + data
    int sent = 0;
    char *ptr = (char *)pkt;
    
    while (sent < total) {
        int n = send(fd, ptr + sent, total - sent, 0);
        if (n <= 0) {
            write_log(runtime_debug, "ERROR", "send failed");
            return ERR_CONN;
        }
        sent += n;
    }
    
    write_log(runtime_debug, "DEBUG", "packet sent");
    return OK;
}

// 接收封包
int recv_packet(int fd, Packet *pkt) {
    if (fd < 0 || !pkt) return ERR_CONN;
    
    // 設定逾時
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timeout, sizeof(timeout));
    
    // 接收標頭
    int n = recv(fd, pkt, sizeof(int) * 3, MSG_WAITALL);  // type + len + client_id
    if (n != sizeof(int) * 3) {
        write_log(runtime_debug, "ERROR", "recv header failed");
        return ERR_CONN;
    }
    
    if (check_packet(pkt) != OK) return ERR_INVALID;
    
    // 接收資料
    if (pkt->len > 0) {
        n = recv(fd, pkt->buf, pkt->len, MSG_WAITALL);
        if (n != pkt->len) {
            write_log(runtime_debug, "ERROR", "recv data failed");
            return ERR_CONN;
        }
    }
    
    write_log(runtime_debug, "DEBUG", "packet received");
    return OK;
}