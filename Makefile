CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -lpthread -g
CLIENT_SOURCES = client/main.cpp
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)
SERVER_SOURCES = server/main.cpp server/client_handle.cpp server/editor.cpp
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
