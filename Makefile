# Convenience commands for the native C build.
BUILD_DIR ?= build
CMAKE ?= cmake

.PHONY: all configure build run test ci clean
all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

run: build
	./$(BUILD_DIR)/crash-bandicoot-pc --disc "$${CRASH_DISC_IMAGE:?Set CRASH_DISC_IMAGE to a USA CUE/BIN image}" $(RUN_ARGS)

test:
	$(CMAKE) -S . -B $(BUILD_DIR) -DBUILD_TESTING=ON
	$(CMAKE) --build $(BUILD_DIR) --parallel
	ctest --test-dir $(BUILD_DIR) --output-on-failure --no-tests=error
	python3 -m unittest tests.test_gool_lisp

ci: test

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean
