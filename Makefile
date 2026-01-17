CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC_DIR = src
BUILD_DIR = build

# Targets for each phase
PHASE1 = $(BUILD_DIR)/phase1_echoserver
PHASE2 = $(BUILD_DIR)/phase2_httpserver
PHASE3 = $(BUILD_DIR)/phase3_staticserver
PHASE4 = $(BUILD_DIR)/phase4_enhancederrorhandling
PHASE5 = $(BUILD_DIR)/phase5_enhancedhttpfeatures

# Default target - build all phases
all: $(BUILD_DIR) $(PHASE1) $(PHASE2) $(PHASE3) $(PHASE4) $(PHASE5)

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

# Build Phase 4: Enhanced Error Handling
$(PHASE4): $(SRC_DIR)/phase4_enhancederrorhandling.c
	$(CC) $(CFLAGS) -o $(PHASE4) $(SRC_DIR)/phase4_enhancederrorhandling.c

# Build Phase 5: Enhanced HTTP Features
$(PHASE5): $(SRC_DIR)/phase5_enhancedhttpfeatures.c
	$(CC) $(CFLAGS) -o $(PHASE5) $(SRC_DIR)/phase5_enhancedhttpfeatures.c

# Individual phase targets
phase1: $(BUILD_DIR) $(PHASE1)

phase2: $(BUILD_DIR) $(PHASE2)

phase3: $(BUILD_DIR) $(PHASE3)

phase4: $(BUILD_DIR) $(PHASE4)

phase5: $(BUILD_DIR) $(PHASE5)

# Run the latest phase (Phase 5)
run: $(PHASE5)
	./$(PHASE5)

# Run specific phases
run-phase1: $(PHASE1)
	./$(PHASE1)

run-phase2: $(PHASE2)
	./$(PHASE2)

run-phase3: $(PHASE3)
	./$(PHASE3)

run-phase4: $(PHASE4)
	./$(PHASE4)

run-phase5: $(PHASE5)
	./$(PHASE5)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean run phase1 phase2 phase3 phase4 phase5 run-phase1 run-phase2 run-phase3 run-phase4 run-phase5
