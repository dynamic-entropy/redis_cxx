## Instructions to run the demo

1. Clone the repository
- Using SSH
```bash
git clone git@github.com:dynamic-entropy/redis_cxx.git
```
- Using HTTPS
```bash
git clone https://github.com/dynamic-entropy/redis_cxx.git
```

2. Start redis server.
You may chose any of the preffered way to run a server. 
https://redis.io/docs/install/

An convenient way is to run a redis server is to use docker
```bash
docker run -d --name redis-stack-server -p 6379:6379 redis/redis-stack-server:latest
```
This will start a docker container with a redis server running on port 6379.

3. Build the project
```bash
make build
```
This will compile the project and build the Producer and Consumer executables.

4. Run the producer
```bash
make run_producer
```
- The producer will prompt you to enter messages.
- You can provide space separated messages as arguments to send as a batch to the redis server. 
- Press `Enter` to send the messages to the redis server.
- Press `Ctrl+C` to stop the producer.

The producer executes LPUSH command against the redis server. https://redis.io/commands/lpush/

5. Run the consumer

5.1 Basic Consumer

```bash
make run_consumer
```
- The consumer reads the messages from the redis server and prints them to the console every 2 seconds.
- Press `Ctrl+C` to stop the consumer.

The consumer executes RPOP command against the redis server. https://redis.io/commands/rpop/ \
RPOP is non-blocking and returns `nil` if the list is empty.

5.2 Blocking Consumer

```bash
make run_blocking_consumer
```
- The consumer blocks and waits for messages to be pushed to the redis server when the list is empty.

The consumer executes BRPOP command against the redis server. https://redis.io/commands/brpop/ \
BRPOP is blocking and waits for a message to be pushed to the list.
