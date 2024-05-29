# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -pthread -lrt

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
TEST_DIR = tests
INCLUDE_DIR = include

# Source files
SRCS_JOB_COMMANDER = $(SRC_DIR)/jobCommander.c $(SRC_DIR)/Qimplementation.c $(SRC_DIR)/functions.c
SRCS_JOB_EXECUTOR = $(SRC_DIR)/jobExecutorServer.c $(SRC_DIR)/Qimplementation.c $(SRC_DIR)/functions.c
# Add any other source files here if needed

# Object files
OBJS_JOB_COMMANDER = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS_JOB_COMMANDER))
OBJS_JOB_EXECUTOR = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS_JOB_EXECUTOR))
# Add any other object files here if needed

# Executable files
EXEC_JOB_COMMANDER = $(BIN_DIR)/jobCommander
EXEC_JOB_EXECUTOR = $(BIN_DIR)/jobExecutorServer
# Add any other executable files here if needed

# Default target
all: directories $(EXEC_JOB_COMMANDER) $(EXEC_JOB_EXECUTOR)

# Create necessary directories
directories:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)

# Compile jobCommander
$(EXEC_JOB_COMMANDER): $(OBJS_JOB_COMMANDER)
	$(CC) $(CFLAGS) -o $@ $^

# Compile jobExecutorServer
$(EXEC_JOB_EXECUTOR): $(OBJS_JOB_EXECUTOR)
	$(CC) $(CFLAGS) -o $@ $^

# Compile jobCommander source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCLUDE_DIR)

# Compile jobExecutorServer source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCLUDE_DIR)

# Run target
run: $(EXEC_JOB_COMMANDER)
	@./$(EXEC_JOB_COMMANDER) linux12.di.uoa.gr 7857 issueJob test 10

# Clean target
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
