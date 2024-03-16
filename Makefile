
BUILD_DIR = build

clean:
	rm -rf $(BUILD_DIR)

build: app/ping.cpp app/producer.cpp
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

run_ping:
	$(BUILD_DIR)/PingRedis

run_producer:
	$(BUILD_DIR)/Producer 

