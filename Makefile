CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC_DIR = src
BUILD_DIR = build

# Targets for each phase
PHASE1 = $(BUILD_DIR)/phase1_echoserver
PHASE2 = $(BUILD_DIR)/phase2_httpserver
PHASE3 = $(BUILD_DIR)/phase3_staticserver

# Default target - build all phases
all: $(BUILD_DIR) $(PHASE1) $(PHASE2) $(PHASE3)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build Phase 1: Echo Server
$(PHASE1): $(SRC_DIR)/phase1_echoserver.c
	$(CC) $(CFLAGS) -o $(PHASE1) $(SRC_DIR)/phase1_echoserver.c

# Build Phase 2: HTTP Server
$(PHASE2): $(SRC_DIR)/phase2_httpserver.c
	$(CC) $(CFLAGS) -o $(PHASE2) $(SRC_DIR)/phase2_httpserver.c

# Build Phase 3: Static File Server
$(PHASE3): $(SRC_DIR)/phase3_staticserver.c
	$(CC) $(CFLAGS) -o $(PHASE3) $(SRC_DIR)/phase3_staticserver.c

# Individual phase targets
phase1: $(BUILD_DIR) $(PHASE1)

phase2: $(BUILD_DIR) $(PHASE2)

phase3: $(BUILD_DIR) $(PHASE3)

# Run the latest phase (Phase 3)
run: $(PHASE3)
	./$(PHASE3)

# Run specific phases
run-phase1: $(PHASE1)
	./$(PHASE1)

run-phase2: $(PHASE2)
	./$(PHASE2)

run-phase3: $(PHASE3)
	./$(PHASE3)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean run phase1 phase2 phase3 run-phase1 run-phase2 run-phase3
