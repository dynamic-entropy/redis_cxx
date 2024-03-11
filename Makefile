
BUILD_DIR = build

clean:
	rm -rf $(BUILD_DIR)

build: app/boost_redis.cpp
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

run_boostredis:
	$(BUILD_DIR)/BoostRedis
