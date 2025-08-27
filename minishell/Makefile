CC = g++

CFLAGS = -Wall -g

TARGET = minishell

SRCS = main.cpp minishell.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean