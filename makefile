# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -I. -pthread -lrt

# Source files
SRCS_JOB_COMMANDER = jobCommander.c Qimplementation.c functions.c
SRCS_JOB_EXECUTOR = jobExecutorServer.c Qimplementation.c functions.c
# Add any other source files here if needed

# Object files
OBJS_JOB_COMMANDER = $(SRCS_JOB_COMMANDER:.c=.o)
OBJS_JOB_EXECUTOR = $(SRCS_JOB_EXECUTOR:.c=.o)
# Add any other object files here if needed

# Executable files
EXEC_JOB_COMMANDER = jobCommander
EXEC_JOB_EXECUTOR = jobExecutorServer
# Add any other executable files here if needed

# Default target
all: $(EXEC_JOB_COMMANDER) $(EXEC_JOB_EXECUTOR)

# Compile jobCommander
$(EXEC_JOB_COMMANDER): $(OBJS_JOB_COMMANDER)
	$(CC) $(CFLAGS) -o $@ $(OBJS_JOB_COMMANDER)

# Compile jobExecutorServer
$(EXEC_JOB_EXECUTOR): $(OBJS_JOB_EXECUTOR)
	$(CC) $(CFLAGS) -o $@ $(OBJS_JOB_EXECUTOR)

# Compile jobCommander source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile jobExecutorServer source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Run target
run: $(EXEC_JOB_COMMANDER)
	@./$(EXEC_JOB_COMMANDER) linux12.di.uoa.gr 7857 issueJob ls
	@./$(EXEC_JOB_COMMANDER) linux12.di.uoa.gr 7857 issueJob ls
	@./$(EXEC_JOB_COMMANDER) linux12.di.uoa.gr 7857 issueJob ls
	@./$(EXEC_JOB_COMMANDER) linux12.di.uoa.gr 7857 issueJob ls
	@./$(EXEC_JOB_COMMANDER) linux12.di.uoa.gr 7857 issueJob ls


# Clean target
clean:
	rm -f $(OBJS_JOB_COMMANDER) $(OBJS_JOB_EXECUTOR) $(EXEC_JOB_COMMANDER) $(EXEC_JOB_EXECUTOR)