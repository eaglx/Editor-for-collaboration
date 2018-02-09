CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -lpthread -g
SERVER_SOURCES = server/main.cpp server/sendrecv.cpp server/serialize.cpp
SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
SERVER_EXEC = server/server.exe

.PHONY: all server

all: server

server: $(SERVER_EXEC)

$(SERVER_EXEC): $(SERVER_OBJECTS)
	$(CC) $(SERVER_OBJECTS) -o $(SERVER_EXEC) $(CFLAGS)

.cpp.o:
	$(CC) -c $^ -o $@ $(CFLAGS)

clean:
	rm server/*.o
	rm server/server.exe

run:
	./server/server.exe
