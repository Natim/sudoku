# Thin wrapper around CMake. Requires libx11-dev.

BUILD_DIR ?= build

all:
	cmake -S . -B $(BUILD_DIR) && cmake --build $(BUILD_DIR)

run: all
	./$(BUILD_DIR)/sudoku

APPLICATIONS_DIR ?= $(HOME)/.local/share/applications

# Sans entree de bureau, GNOME Shell ne sait pas a quelle application
# appartient la fenetre et lui donne une icone generique dans le dock.
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
