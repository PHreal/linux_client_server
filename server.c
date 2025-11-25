#include "common.h"
#include <sys/wait.h>
#include <signal.h>
#include <sys/sysinfo.h>

static volatile int child_count = 0;

void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        child_count--;
    }
}

void handle_sigint(int sig) {
    write_log(runtime_debug, "INFO", "Server shutting down");
    exit(0);
}

// 取得系統資訊
void fetch_sysinfo(char *buf, size_t size) {
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        snprintf(buf, size,
                "Uptime: %ld sec\n"
                "Total RAM: %lu MB\n"
                "Free RAM: %lu MB\n"
                "Processes: %d\n",
                si.uptime,
                si.totalram / (1024 * 1024),
                si.freeram / (1024 * 1024),
                si.procs);
    }
}

// 取得 CPU 資訊
void fetch_cpuinfo(char *buf, size_t size) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        snprintf(buf, size, "Cannot read CPU info\n");
        return;
    }
    
    char line[256];
    int cores = 0;
    char model[256] = {0};
    
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "processor", 9) == 0) cores++;
        else if (strncmp(line, "model name", 10) == 0 && !model[0]) {
            char *p = strchr(line, ':');
            if (p) {
                strncpy(model, p + 2, sizeof(model) - 1);
                model[strcspn(model, "\n")] = 0;
            }
        }
    }
    fclose(fp);
    
    snprintf(buf, size, "CPU cores: %d\nModel: %s\n", cores, model);
}

// 取得記憶體資訊
void fetch_meminfo(char *buf, size_t size) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        snprintf(buf, size, "Cannot read memory info\n");
        return;
    }
    
    char line[256];
    long total = 0, avail = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "MemTotal: %ld kB", &total);
        sscanf(line, "MemAvailable: %ld kB", &avail);
    }
    fclose(fp);
    
    snprintf(buf, size,
            "Total: %ld MB\n"
            "Available: %ld MB\n"
            "Used: %ld MB\n"
            "Usage: %.1f%%\n",
            total / 1024, avail / 1024, (total - avail) / 1024,
            (total - avail) * 100.0 / total);
}

// 處理客戶端
void process_client(int fd, int client_num) {
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Child process handling Client #%d [PID: %d]", 
             client_num, getpid());
    write_log(runtime_debug, "INFO", log_msg);
    
    Packet req, resp;
    
    while (1) {
        if (recv_packet(fd, &req) != OK) break;
        
        snprintf(log_msg, sizeof(log_msg), "Client #%d request type: %d", 
                 req.client_id, req.type);
        write_log(runtime_debug, "INFO", log_msg);
        
        memset(&resp, 0, sizeof(resp));
        resp.type = req.type;
        resp.client_id = req.client_id;
        
        switch (req.type) {
            case REQ_SYSINFO:
                fetch_sysinfo(resp.buf, sizeof(resp.buf));
                break;
            case REQ_CPUINFO:
                fetch_cpuinfo(resp.buf, sizeof(resp.buf));
                break;
            case REQ_MEMINFO:
                fetch_meminfo(resp.buf, sizeof(resp.buf));
                break;
        }
        
        resp.len = strlen(resp.buf);
        
        snprintf(log_msg, sizeof(log_msg), "Responding to Client #%d", req.client_id);
        write_log(runtime_debug, "INFO", log_msg);
        
        if (send_packet(fd, &resp) != OK) break;
    }
    
    snprintf(log_msg, sizeof(log_msg), "Client #%d disconnected", req.client_id);
    write_log(runtime_debug, "INFO", log_msg);
    
    close(fd);
    exit(0);
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            runtime_debug = 1;
        }
    }
    
    printf("Compile-time debug: %s\n", COMPILE_DEBUG ? "ON" : "OFF");
    printf("Runtime debug: %s\n", runtime_debug ? "ON" : "OFF");
    
    signal(SIGCHLD, handle_sigchld);
    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, SIG_IGN);
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);
    
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(1);
    }
    
    if (listen(server_fd, MAX_CONN) < 0) {
        perror("listen failed");
        exit(1);
    }
    
    write_log(runtime_debug, "INFO", "Server started on port 8888");
    
    int client_counter = 0;  // 客戶端計數器
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd < 0) continue;
        
        if (child_count >= MAX_CONN) {
            write_log(runtime_debug, "WARN", "Max connections reached");
            close(client_fd);
            continue;
        }
        
        pid_t pid = fork();
        if (pid < 0) {
            write_log(runtime_debug, "ERROR", "fork failed");
            close(client_fd);
            continue;
        }
        
        if (pid == 0) {
            close(server_fd);
            process_client(client_fd, client_counter);
        } else {
            child_count++;
            client_counter++;
            close(client_fd);
            
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), 
                     "Client #%d connected [PID: %d], Active: %d", 
                     client_counter, pid, child_count);
            write_log(runtime_debug, "INFO", log_msg);
        }
    }
    
    return 0;
}