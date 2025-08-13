# コンパイラとフラグの設定
# 変数　＝　value
CC = gcc								# コンパイル方法
CFLAGS = -Wall -g
INCLUDE = -Icommon -Iserver -Iclient

# ディレクトリの定義
COMMON_DIR = common
SERVER_DIR = server
CLIENT_DIR = client

# 共通ファイル
COMMON_SRCS = $(COMMON_DIR)/board.c $(COMMON_DIR)/clearBuffer.c $(COMMON_DIR)/net.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)

# サーバーファイル
SERVER_SRCS = $(SERVER_DIR)/game.c $(SERVER_DIR)/server.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)

# クライアントファイル
CLIENT_SRCS = $(CLIENT_DIR)/client.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

# ターゲットの定義
all: myserver myclient

myserver: $(COMMON_OBJS) $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o myserver $(COMMON_OBJS) $(SERVER_OBJS)

myclient: $(COMMON_OBJS) $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o myclient $(COMMON_OBJS) $(CLIENT_OBJS)

# オブジェクトファイルのルール
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# クリーンアップのルール
clean:
	rm -f $(COMMON_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS) myserver myclient

.PHONY: all clean
