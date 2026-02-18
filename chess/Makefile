BUILD_DIR := build
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=$(if $(RELEASE),Release,Debug)

.PHONY: all build test clean

all: build

build:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)
	cmake --build $(BUILD_DIR)

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
