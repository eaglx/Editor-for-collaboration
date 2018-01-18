CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -lpthread -g
CLIENT_SOURCES = client/main.cpp
SERVER_SOURCES = server/main.cpp server/client_handle.cpp server/editor.cpp
CLIENT_EXEC = client/client
SERVER_EXEC = server/server

.PHONY: all client server

all: client server

client: $(CLIENT_EXEC)
server: $(SERVER_EXEC)

$(CLIENT_EXEC):
	$(CC) $(CLIENT_SOURCES) -o $@ $(CFLAGS)

$(SERVER_EXEC):
	$(CC) $(SERVER_SOURCES) -o $@ $(CFLAGS)
