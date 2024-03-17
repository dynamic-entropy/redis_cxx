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
- Press `Enter` to send the messages to the redis server.
- Press `Enter` without messages to stop the producer.

The producer executes LPUSH command against the redis server. https://redis.io/commands/lpush/

5. Run the consumer

```bash
make run_consumer
```
- The consumer reads the messages from the redis server and prints them to the console
- The consumer blocks and waits for messages to be pushed to the redis server when the list is empty.
- Press `Ctrl+C` to stop the consumer.

The consumer executes BRPOP command against the redis server. https://redis.io/commands/brpop/ \
BRPOP is blocking and waits for a message to be pushed to the list.

Alternatively a non-blocking variant of the consumer can be run.
RPOP is non-blocking and returns `nil` if the list is empty. https://redis.io/commands/rpop/ \


## 2 Phase Commit Queue

The producer and consumer can be run in 2 phase commit mode. This is a very basic implementation of 2 phase commit protocol.
It protects against loss of messages when the consumer is not able to process the message and its queues are filling up.

### 2PC_Producer

1. The producer will store the message in a redis list names `topic:pending`.
   - This is done with the `LPUSH` command.

2. The producer performs the necessary operations and perform the following operations in a transaction.
    - If the operation fails:
        - Pops the key from the redis store
        - This is done with the `RPOP` command.
    - If the operation is successful:
        - Moves the key from `topic:pending` to `topic:committed` list.
        - This is done with the `LMOVE topic:pending topic:committed RIGHT LEFT` command.

### 2PC_Consumer

1. The consumer reads messages from the list `topic:committed` and performs the necessary operations.
    - This is done with the `LRANGE topic:committed -1 -1` command.
2. If the operation is successful, the consumer acknowledges the message by deleting the message from the list.
    - This is done with the `RPOP` command.
3. If the operation fails, the message is left in the list and the consumer will try to process the message again in the next iteration.


#### Limitations
Since the producer and consumer use a simple queue as a datastructure, individual messages are not addressable this means
- The processing and delievery of messages has to be **in order**
   - This effects scalability and is a reason why this implementation is not suitable for large scale systems at all.
   
- Multiple producers cannot produce to the same topic
    - Else a producer may ack a message that was not produced by itself.
- Similarly only a single consumer can consumer the messages synchronously and in order.
> A Locking mechanism can help with some of the problems and allow multiple producer and consumers to work on the same topic.
- A redis instance does not provide isolation of memory failures, since there is no way to partition memory among different topics.
    - This means a misbehaving topic can fill up the memory and prevent other topics from working as well.
> This can be helped with programatic monitoring of the length of the queue
  