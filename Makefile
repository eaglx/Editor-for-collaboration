CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -lpthread -g
CLIENT_SOURCES = client/main.cpp client/manage_editor.cpp client/manage_activ.cpp
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)
SERVER_SOURCES = server/main.cpp server/client_handle_editor.cpp server/client_handle_activ.cpp server/control_client.cpp server/control_clientACA.cpp server/accept_connections_ed.cpp server/accept_connections_activ.cpp
SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENT_EXEC = client/client
SERVER_EXEC = server/server

.PHONY: all client server

all: client server

client: $(CLIENT_EXEC)
server: $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_OBJECTS)
	$(CC) $(CLIENT_OBJECTS) -o $(CLIENT_EXEC) $(CFLAGS)

$(SERVER_EXEC): $(SERVER_OBJECTS)
	$(CC) $(SERVER_OBJECTS) -o $(SERVER_EXEC) $(CFLAGS)

.cpp.o:
	$(CC) -c $^ -o $@ $(CFLAGS)

clean:
	rm client/*.o
	rm server/*.o
