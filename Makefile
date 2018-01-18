CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -lpthread -g
CLIENT_SOURCES = client/main.cpp client/manage_editor.cpp client/manage_activ.cpp
SERVER_SOURCES = server/main.cpp server/client_handle_editor.cpp server/client_handle_activ.cpp server/control_client.cpp server/control_clientACA.cpp server/accept_connections_ed.cpp server/accept_connections_activ.cpp
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
