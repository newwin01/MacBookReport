CC = gcc
CFLAGS = -Wall -pthread

SERVER_TARGET = dirup_server
CLIENT_TARGET = dirup_client  
SRCS1 = dirup_server.c lib/helper.c 
SRCS2 = dirup_client.c lib/helper.c 
OBJS1 = $(SRCS1:.c=.o)
OBJS2 = $(SRCS2:.c=.o)

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)  # Compile both server and client targets

$(SERVER_TARGET): $(OBJS1)
	$(CC) $(CFLAGS) -o $@ $^ 

$(CLIENT_TARGET): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $^ 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -std=gnu99

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(OBJS1) $(OBJS2)
