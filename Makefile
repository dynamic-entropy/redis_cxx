
BUILD_DIR = build

clean:
	rm -rf $(BUILD_DIR)

build: app/consumer.cpp app/producer.cpp app/blocking_consumer.cpp
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

run_producer:
	$(BUILD_DIR)/Producer 

run_consumer:
	$(BUILD_DIR)/Consumer

run_blocking_consumer:
	$(BUILD_DIR)/BlockingConsumer
