# Thin wrapper around CMake. Requires libx11-dev.

BUILD_DIR ?= build

all:
	cmake -S . -B $(BUILD_DIR) && cmake --build $(BUILD_DIR)

run: all
	./$(BUILD_DIR)/sudoku

# Checks and helpers from scripts/, built and run one at a time. Arguments go
# through ARGS, as in: make verify-celebration ARGS="2500 120"
CHECKS = verify-bitmap verify-celebration verify-generator write-sample-grid

$(CHECKS):
	cmake -S . -B $(BUILD_DIR) && cmake --build $(BUILD_DIR) --target $(subst -,_,$@)
	./$(BUILD_DIR)/$(subst -,_,$@) $(ARGS)

# The .deb and the archive published by the release workflow, built apart so
# that the packaging prefix stays out of the development build.
PACKAGE_DIR ?= build-package

package:
	cmake -S . -B $(PACKAGE_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr $(if $(VERSION),-DSUDOKU_VERSION=$(VERSION))
	cmake --build $(PACKAGE_DIR)
	cd $(PACKAGE_DIR) && cpack -G "DEB;TGZ"

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
	rm -rf $(BUILD_DIR) $(PACKAGE_DIR)

.PHONY: all run package desktop desktop-uninstall clean mrproper $(CHECKS)
