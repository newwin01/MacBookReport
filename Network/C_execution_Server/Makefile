CC = gcc
CFLAGS = -Wall -pthread

SERVER_TARGET = oss_server
CLIENT_TARGET = oss_client  # Add a new target for the client
SRCS1 = oss_server.c lib/helper.c  # Include oss_client.c in the source files
SRCS2 = oss_client.c
OBJS1 = $(SRCS1:.c=.o)
OBJS2 = $(SRCS2:.c=.o)

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)  # Compile both server and client targets

$(SERVER_TARGET): $(OBJS1)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_TARGET): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(OBJS1) $(OBJS2)
