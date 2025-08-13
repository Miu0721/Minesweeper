# Makefile (flat layout)
CC = gcc
CFLAGS = -Wall -Wextra -O2 -MMD -MP

# ⬇️ EDIT these to match your files in the repo root
COMMON_SRCS := board.c clearBuffer.c net.c
SERVER_SRCS := server.c game.c $(COMMON_SRCS)
CLIENT_SRCS := client.c $(COMMON_SRCS)

SERVER_OBJS := $(SERVER_SRCS:.c=.o)
CLIENT_OBJS := $(CLIENT_SRCS:.c=.o)

all: myserver myclient

myserver: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS)

myclient: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) myserver myclient
	rm -f $(SERVER_OBJS:.o=.d) $(CLIENT_OBJS:.o=.d)

-include $(SERVER_OBJS:.o=.d) $(CLIENT_OBJS:.o=.d)
