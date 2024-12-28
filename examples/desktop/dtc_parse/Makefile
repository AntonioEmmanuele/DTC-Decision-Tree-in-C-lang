# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I../../../src

# Directories
SRC_DIR = ../../../src
EXAMPLE_DIR = .
OBJ_DIR = $(EXAMPLE_DIR)/obj

# Source files
SRC_FILES = $(SRC_DIR)/tree_visit.c
MAIN_FILE = $(EXAMPLE_DIR)/main.c

# Object files
OBJ_FILES = $(OBJ_DIR)/tree_visit.o $(OBJ_DIR)/main.o

# Output binary
TARGET = main

# Default rule
all: $(TARGET)

# Build target
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files into obj/ directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(EXAMPLE_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
