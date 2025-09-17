# 编译器与选项
CC      := gcc
CFLAGS  := -O3 -fopenmp -Wall -Wextra -Wa,--noexecstack
LDFLAGS := -Wl,-z,noexecstack
LDLIBS  := -lm

# 目标
TARGET  := TriRNASP
SRC     := TriRNASP.c
OBJ     := $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
