.PHONY: all build clean test run lint format

BUILD_DIR := build
BUILD_TYPE ?= Release

all: build

build:
	cmake -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build $(BUILD_DIR) --parallel

clean:
	cmake --build $(BUILD_DIR) --target clean

test:
	cd $(BUILD_DIR) && ctest --output-on-failure

run:
	./$(BUILD_DIR)/inventory_management_system

lint:
	clang-tidy src/**/*.cpp -- -std=c++20

format:
	clang-format -i src/**/*.cpp src/**/*.hpp

debug:
	$(MAKE) build BUILD_TYPE=Debug
