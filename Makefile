CFLAGS = -Wall -g -Werror -Wno-error=unused-variable
CC = g++

all: server subscriber

server: server.cpp run_server.cpp common.cpp
	$(CC) $(CFLAGS) -o server server.cpp run_server.cpp common.cpp

subscriber: client.cpp run_client.cpp common.cpp
	$(CC) $(CFLAGS) -o subscriber client.cpp run_client.cpp common.cpp

clean:
	rm -f *.o server subscriber