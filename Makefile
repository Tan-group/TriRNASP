# =========================
# TriRNASP Makefile (Optimized)
# =========================

# Compiler
CC      := gcc

# Compilation flags
# -O3             : High-level optimization
# -march=native   : Optimize for the local CPU architecture
# -ffast-math     : Enable fast math (not strictly IEEE-compliant)
# -fno-math-errno : Skip setting errno for math functions
# -fopenmp        : Enable OpenMP multithreading
# -Wall -Wextra   : Show all warnings
# -Wa,--noexecstack : Prevent executable stack at compile time
CFLAGS  := -O3 -march=native -ffast-math -fno-math-errno -fopenmp -Wall -Wextra -Wa,--noexecstack

# Linking flags
# -lm             : Link math library
# -fopenmp        : Link OpenMP library
# -Wl,-z,noexecstack : Prevent executable stack at link time
LDFLAGS := -lm -fopenmp -Wl,-z,noexecstack

# Target executable
TARGET  := TriRNASP

# Source files (add more .c files here if needed)
SRC     := TriRNASP.c
OBJ     := $(SRC:.c=.o)

# Default target
all: $(TARGET) datasets

# Link all object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .c file into an object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Automatically unzip any dataset archives (*.zip)
datasets:
	@echo ">>> Unzipping datasets..."
	@for f in *.zip; do \
		if [ -f "$$f" ]; then \
			echo "   - $$f"; \
			unzip -o -q "$$f" -d .; \
		fi; \
	done
	@echo ">>> Done."

# Clean build artifacts
clean:
	rm -f $(OBJ) $(TARGET)
