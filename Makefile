# Thin wrapper around CMake. Requires libx11-dev.

BUILD_DIR ?= build

all:
	cmake -S . -B $(BUILD_DIR) && cmake --build $(BUILD_DIR)

run: all
	./$(BUILD_DIR)/sudoku

clean:
	cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true

mrproper:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean mrproper
