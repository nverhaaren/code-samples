BUILD_DIR     := build
BUILD_COV_DIR := build-cov
CMAKE_FLAGS   := -DCMAKE_BUILD_TYPE=$(if $(RELEASE),Release,Debug)

.PHONY: all build test coverage clean

all: build

build:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)
	cmake --build $(BUILD_DIR)

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

coverage:
	cmake -S . -B $(BUILD_COV_DIR) -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
	cmake --build $(BUILD_COV_DIR)
	cd $(BUILD_COV_DIR) && ctest --output-on-failure
	lcov --capture --directory $(BUILD_COV_DIR) --output-file coverage.info
	lcov --remove coverage.info '/usr/*' '*/Catch2/*' --output-file coverage_filtered.info
	genhtml coverage_filtered.info --output-directory coverage-report
	@echo "Coverage report: coverage-report/index.html"

clean:
	rm -rf $(BUILD_DIR) $(BUILD_COV_DIR) coverage.info coverage_filtered.info coverage-report
