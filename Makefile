
BUILD_DIR = build

clean:
	rm -rf $(BUILD_DIR)

build:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

run_producer: build
	$(BUILD_DIR)/Producer 

run_consumer: build
	$(BUILD_DIR)/Consumer
