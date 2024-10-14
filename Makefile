# Makefile

# Compiler
CC = gcc

# Source directory
SRC_DIR = src

# Output directory
BIN_DIR = output

# Source files
SRC = $(SRC_DIR)/logParser.c

# Output file
OUT = $(BIN_DIR)/logParser

# Default target
all: $(OUT)

# Rule to build the output
$(OUT): $(SRC)
	@mkdir -p $(BIN_DIR)
	$(CC) $(SRC) -o $(OUT)

# Clean target
clean:
	rm -f $(OUT)