# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -IProtocol -IAccount -IMovie

# Source and binary names
SERVER_SRC = server.c Protocol/protocol.c Movie/movie.c Account/account.c
CLIENT_SRC = client.c Protocol/protocol.c
SERVER_BIN = server
CLIENT_BIN = client

# Default IP if none is provided
DEFAULT_IP = 127.0.0.1
IP = $(or $(word 2, $(MAKECMDGOALS)), $(DEFAULT_IP))

# Targets
.PHONY: all clean run_server run_client

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SERVER_SRC)

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRC)

run_server: $(SERVER_BIN)
	./$(SERVER_BIN)

run_client: $(CLIENT_BIN)
	./$(CLIENT_BIN) $(IP)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) *.o log.txt 
