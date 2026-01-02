CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC_DIR = src
BUILD_DIR = build

# Targets for each phase
PHASE1 = $(BUILD_DIR)/phase1_echoserver
PHASE2 = $(BUILD_DIR)/phase2_httpserver

# Default target - build all phases
all: $(BUILD_DIR) $(PHASE1) $(PHASE2)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build Phase 1: Echo Server
$(PHASE1): $(SRC_DIR)/phase1_echoserver.c
	$(CC) $(CFLAGS) -o $(PHASE1) $(SRC_DIR)/phase1_echoserver.c

# Build Phase 2: HTTP Server
$(PHASE2): $(SRC_DIR)/phase2_httpserver.c
	$(CC) $(CFLAGS) -o $(PHASE2) $(SRC_DIR)/phase2_httpserver.c

# Individual phase targets
phase1: $(BUILD_DIR) $(PHASE1)

phase2: $(BUILD_DIR) $(PHASE2)

# Run the latest phase (Phase 2)
run: $(PHASE2)
	./$(PHASE2)

# Run specific phases
run-phase1: $(PHASE1)
	./$(PHASE1)

run-phase2: $(PHASE2)
	./$(PHASE2)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean run phase1 phase2 run-phase1 run-phase2
