CC=clang
TARGET=ass1

.PHONY: all
all:
	@echo "Building the program"
	mkdir -p build
	cd build && cmake .. && make

.PHONY: run
run:
	@echo "Running the program"
	cd build && cmake --build . && ./ass1