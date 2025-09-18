CC      := gcc
CFLAGS  := -O3 -fopenmp -Wall -Wextra -Wa,--noexecstack
LDFLAGS := -lm -Wl,-z,noexecstack

TARGET  := TriRNASP
SRC     := TriRNASP.c
OBJ     := $(SRC:.c=.o)

all: $(TARGET) datasets

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

datasets:
	@echo ">>> Unzipping datasets..."
	@for f in *.zip; do \
		echo "   - $$f"; \
		unzip -o -q $$f -d .; \
	done
	@echo ">>> Done."

clean:
	rm -f $(OBJ) $(TARGET)
