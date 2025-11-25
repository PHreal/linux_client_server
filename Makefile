CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -L. -lcommon

# 取消註解啟用編譯時除錯
# CFLAGS += -DDEBUG_COMPILE

SERVER = server
CLIENT = client
LIB = libcommon.so

all: $(LIB) $(SERVER) $(CLIENT)

$(LIB): common.o
	$(CC) -shared -o $@ $^

$(SERVER): server.o $(LIB)
	$(CC) -o $@ server.o $(LDFLAGS)

$(CLIENT): client.o $(LIB)
	$(CC) -o $@ client.o $(LDFLAGS)

%.o: %.c common.h
	$(CC) $(CFLAGS) -c $<

debug: CFLAGS += -DDEBUG_COMPILE -g
debug: clean all

clean:
	rm -f $(SERVER) $(CLIENT) $(LIB) *.o

run-server:
	LD_LIBRARY_PATH=. ./$(SERVER)

run-server-debug:
	LD_LIBRARY_PATH=. ./$(SERVER) --debug

run-client:
	LD_LIBRARY_PATH=. ./$(CLIENT)

run-client-debug:
	LD_LIBRARY_PATH=. ./$(CLIENT) --debug

.PHONY: all debug clean run-server run-server-debug run-client run-client-debug