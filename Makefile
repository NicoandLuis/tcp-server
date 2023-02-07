BINARY_NAME=threaded_server.out

# File locations
OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))\
		$(patsubst $(SRC_DIR)/%.S,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.S))
HEADERS=$(shell find $(SRC_DIR) -name "*.h")

# Compiler settings
CC=cc
CFLAGS=-g -Wall -fsanitize=address
LDFLAGS=-lpthread -g -Wall -fsanitize=address

SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin

.PHONY: build clean clean-build
build: $(BIN_DIR)/$(BINARY_NAME)

clean: 
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

clean-build: clean
	$(MAKE) build

$(BIN_DIR)/$(BINARY_NAME): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $+

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S | $(OBJ_DIR)
	nasm -f elf64 -o $@ $<

$(BIN_DIR):
	mkdir $(BIN_DIR)
	
$(OBJ_DIR):
	mkdir $(OBJ_DIR)
