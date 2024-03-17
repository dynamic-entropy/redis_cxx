
BUILD_DIR = build

clean:
	rm -rf $(BUILD_DIR)

build: app/ping.cpp app/producer.cpp app/consumer.cpp app/2pc_producer.cpp
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

run_ping:
	$(BUILD_DIR)/PingRedis

run_producer:
	$(BUILD_DIR)/Producer 

run_consumer:
	$(BUILD_DIR)/Consumer 

run_2pc_producer:
	$(BUILD_DIR)/2PC_Producer

