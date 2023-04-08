# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -lpthread

# Source files
SERVER_SRCS = server.c packets.c DLLNode.c
CLIENT_SRCS = client.c packets.c
SRCS = $(SERVER_SRCS) $(CLIENT_SRCS)

# Header files
HDRS = server.h packets.h DLLNode.h client.h

# Object files
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
OBJS = $(SERVER_OBJS) $(CLIENT_OBJS)

# Executable files
SERVER_EXEC = server
CLIENT_EXEC = client

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o $(CLIENT_EXEC)

# Compilation rules
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(SERVER_EXEC) $(CLIENT_EXEC)
