# Redis-Lite

Redis-Lite is a learning-focused, Redis-inspired in-memory key–value store written in C++.  
This project was built to understand what *backend actually means* beyond APIs and databases.

While working on backend web applications, I realized that although I could build APIs and connect databases, I did not clearly understand backend fundamentals like concurrency, CPU usage, race conditions, latency, and scalability.  
Redis was often mentioned as a “cache”, but how it really works internally and why it improves performance was not clear to me.

So I decided to build a minimal Redis-like system from scratch to understand:
- Why Redis keeps data in memory
- Why Redis follows a single-threaded execution model
- How serialization prevents race conditions
- How latency is reduced by avoiding repeated database access

Redis-Lite stores data in memory using an unordered_map and processes all operations through a single worker thread.  
Multiple client threads can send requests, but only one worker thread executes them, ensuring correctness and avoiding race conditions.

The core idea is simple:  
**mutex alone does not guarantee correctness, design does**.

Architecture (conceptual flow):

+--------------------+
|   Client Threads   |
| (SET / GET / DEL)  |
+---------+----------+
          |
          | push Command
          v
+--------------------+
|   Command Queue    |   (protected by mutex)
|     FIFO Queue     |
+---------+----------+
          |
          | notify_one()
          v
+--------------------+
|   Worker Thread    |
|   (workerLoop)    |
+---------+----------+
          |
          | serialized execution
          v
+--------------------+
|   In-Memory Store  |
| unordered_map<K,V>|
+--------------------+
          |
          | (GET result via promise/future)
          v
+--------------------+
|  Client Thread     |
+--------------------+

Each client request (SET, GET, DEL) is converted into a Command object and pushed into a thread-safe queue.  
A single worker thread continuously waits on a condition variable, pops commands one by one, and executes them serially.  
Because only the worker thread accesses the data store, race conditions are avoided without complex locking logic.

For GET operations, std::promise and std::future are used so that the client thread can safely wait for the result produced by the worker thread.

This design is inspired by Redis’s internal model where correctness and predictability are preferred over uncontrolled parallelism.

Example usage:

RedisLite redis;

std::thread t1([&](){ redis.set("user1", "amit"); });
std::thread t2([&](){ redis.set("user2", "sanjay"); });

t1.join();
t2.join();

std::thread t3([&](){
    std::cout << redis.get("user1") << std::endl;
});

t3.join();

Build and run (C++17):

g++ -std=c++17 main.cpp RedisLite.cpp -o redis-lite
./redis-lite

This project is intentionally minimal and educational.  
It does not implement networking, persistence, TTL, or advanced Redis data structures.  
The focus is purely on understanding concurrency, serialization, and in-memory data access.

By building Redis-Lite, I now clearly understand why Redis exists, how it reduces latency compared to database calls, and how backend scalability problems are solved at the system-design level rather than just frameworks.

This project represents my transition from  
“I can build backend apps”  
to  
“I understand how backend systems actually work.”
