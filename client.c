#include "common.h"

int client_id = 0;  // 客戶端 ID

void show_menu(void) {
    printf("\n===== System Monitor (Client #%d) =====\n", client_id);
    printf("1. System Info\n");
    printf("2. CPU Info\n");
    printf("3. Memory Info\n");
    printf("0. Exit\n");
    printf("========================================\n");
    printf("Choice: ");
}

int main(int argc, char *argv[]) {
    char *ip = "127.0.0.1";
    
    // 生成唯一的客戶端 ID（使用程序 ID）
    client_id = getpid() % 10000;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            runtime_debug = 1;
        } else if (strcmp(argv[i], "--server") == 0 && i + 1 < argc) {
            ip = argv[++i];
        } else if (strcmp(argv[i], "--id") == 0 && i + 1 < argc) {
            client_id = atoi(argv[++i]);
        }
    }
    
    printf("========================================\n");
    printf("Client ID: %d\n", client_id);
    printf("Compile-time debug: %s\n", COMPILE_DEBUG ? "ON" : "OFF");
    printf("Runtime debug: %s\n", runtime_debug ? "ON" : "OFF");
    printf("Connecting to %s:%d\n", ip, SERVER_PORT);
    printf("========================================\n\n");
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket failed");
        exit(1);
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect failed");
        exit(1);
    }
    
    printf("Client #%d connected!\n", client_id);
    
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Client #%d connected to server", client_id);
    write_log(runtime_debug, "INFO", log_msg);
    
    Packet req, resp;
    int choice;
    
    while (1) {
        show_menu();
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }
        
        if (choice == 0) break;
        if (choice < 1 || choice > 3) continue;
        
        memset(&req, 0, sizeof(req));
        req.type = choice;
        req.len = 0;
        
        if (send_packet(fd, &req) != OK) {
            printf("Send failed!\n");
            break;
        }
        
        if (recv_packet(fd, &resp) != OK) {
            printf("Receive failed!\n");
            break;
        }
        
        printf("\n----- Response -----\n");
        printf("%s", resp.buf);
        printf("-------------------\n");
    }
    
    close(fd);
    
    snprintf(log_msg, sizeof(log_msg), "Client #%d closed", client_id);
    write_log(runtime_debug, "INFO", log_msg);
    return 0;
}