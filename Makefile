# Thin wrapper around CMake. Requires libx11-dev.

BUILD_DIR ?= build

all:
	cmake -S . -B $(BUILD_DIR) && cmake --build $(BUILD_DIR)

run: all
	./$(BUILD_DIR)/sudoku

APPLICATIONS_DIR ?= $(HOME)/.local/share/applications

# Without a desktop entry, GNOME Shell cannot tell which application owns the
# window and gives it a generic icon in the dock.
desktop: all
	install -Dm644 $(BUILD_DIR)/sudoku.desktop $(APPLICATIONS_DIR)/sudoku.desktop
	update-desktop-database $(APPLICATIONS_DIR) 2>/dev/null || true

desktop-uninstall:
	rm -f $(APPLICATIONS_DIR)/sudoku.desktop
	update-desktop-database $(APPLICATIONS_DIR) 2>/dev/null || true

clean:
	cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true

mrproper:
	rm -rf $(BUILD_DIR)

.PHONY: all run desktop desktop-uninstall clean mrproper
