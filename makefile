# Makefile for COS344 Homework - Uses CMake build system
# Run: make build, make run, or make all

BUILD_DIR = out/build/x64-debug
EXECUTABLE = $(BUILD_DIR)/HomwWorkAssignment.exe

.PHONY: build run clean all help

help:
	@echo "Available targets:"
	@echo "  make build   - Build the project using CMake"
	@echo "  make run     - Run the executable"
	@echo "  make all     - Build and run"
	@echo "  make clean   - Clean build artifacts"

build: CMakeLists.txt
	@echo "Building project with CMake..."
	cmake --build $(BUILD_DIR) --config Debug

$(EXECUTABLE): build
	@echo "Executable ready: $(EXECUTABLE)"

run: $(EXECUTABLE)
	@echo "Running HomwWorkAssignment.exe..."
	$(EXECUTABLE)

all: build run

clean:
	@echo "Cleaning build artifacts..."
	cmake --build $(BUILD_DIR) --target clean
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)