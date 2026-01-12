# Redis-Lite

A lightweight, Redis-inspired in-memory key–value store built in modern C++ to explore backend systems design, concurrency patterns, and thread synchronization.

## Overview

Redis-Lite implements a simplified version of Redis's single-threaded command execution model using a producer–consumer architecture. Multiple client threads can concurrently submit `SET`, `GET`, and `DEL` commands, which are processed serially by a single worker thread—mirroring Redis's approach to maintaining consistency without complex locking on the data structure itself.

This project was built as a **learning exercise** to understand:
- Why single-threaded command execution can be faster than multi-threaded approaches for in-memory data stores
- How to synchronize concurrent producers with a single consumer
- Practical use of C++ synchronization primitives (`std::mutex`, `std::condition_variable`, `std::promise`, `std::future`)
- Performance characteristics of lock-based concurrency patterns

**Note:** This is an educational project. It does not include networking, persistence, or the full Redis command set.

---

## Key Features

- ✅ **Single-Threaded Command Execution**: Commands execute serially in a dedicated worker thread, avoiding race conditions on the data structure
- ✅ **Producer–Consumer Architecture**: Client threads enqueue commands; a single worker dequeues and executes them
- ✅ **Synchronous GET Support**: Uses `std::promise` / `std::future` to return results to the calling thread
- ✅ **Thread-Safe Command Queue**: Synchronized with `std::mutex` and `std::condition_variable`
- ✅ **Stress Tested**: Handles 10,000 concurrent `SET` requests in ~700 ms on a local machine
- ✅ **Clean C++17**: No external dependencies; uses STL containers and threading primitives

---

## Architecture

### Why Single-Threaded Execution?

Redis uses a single-threaded event loop because:
1. **Avoids lock contention** on every data structure operation
2. **Simplifies consistency** (no complex multi-threaded race conditions)
3. **CPU-bound operations are fast** in memory; the bottleneck is I/O, not CPU

Redis-Lite adopts the same philosophy: **multiple producers** (client threads) submit commands to a queue, and a **single consumer** (worker thread) processes them sequentially.

### High-Level Design

```
┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│  Client     │  │  Client     │  │  Client     │
│  Thread 1   │  │  Thread 2   │  │  Thread N   │
└──────┬──────┘  └──────┬──────┘  └──────┬──────┘
       │                │                │
       │  SET key val   │  GET key       │  DEL key
       └────────┬───────┴────────┬───────┘
                │                │
                ▼                ▼
         ┌──────────────────────────┐
         │   Thread-Safe Queue      │  ◄── std::mutex + std::condition_variable
         │  (std::queue<Command>)   │
         └────────────┬─────────────┘
                      │
                      ▼
              ┌───────────────┐
              │ Worker Thread │  ◄── Single consumer
              │  (Event Loop) │
              └───────┬───────┘
                      │
                      ▼
         ┌────────────────────────┐
         │   std::unordered_map   │  ◄── In-memory key–value store
         │    <string, string>    │       (no locks needed here!)
         └────────────────────────┘
```

---

## How It Works

### Step-by-Step Execution Flow

1. **Client threads** call `SET(key, value)`, `GET(key)`, or `DEL(key)` via the public API
2. **Commands are wrapped** in a `Command` struct and pushed into a thread-safe queue
3. **Worker thread wakes up** (via `condition_variable`) and dequeues the command
4. **Command executes serially** on the `std::unordered_map`
   - `SET` and `DEL` complete immediately (no return value needed)
   - `GET` stores the result in a `std::promise`, which the client thread retrieves via `std::future`
5. **Repeat**: The worker loops indefinitely, processing the next command

### Why `std::promise` / `std::future` for GET?

- `SET` and `DEL` are fire-and-forget (client doesn't need a response)
- `GET` must **block the calling thread** until the worker retrieves the value
- `std::promise` (set by worker) + `std::future` (retrieved by client) provides a clean synchronization point

---

## Public API

```cpp
class RedisLite {
public:
    RedisLite();                              // Starts worker thread
    ~RedisLite();                             // Stops worker thread gracefully

    void SET(const std::string& key, const std::string& value);
    std::string GET(const std::string& key);  // Blocks until result is ready
    void DEL(const std::string& key);
};
```

### Example Usage

```cpp
#include "RedisLite.h"
#include <iostream>

int main() {
    RedisLite redis;

    redis.SET("user:100", "Alice");
    redis.SET("user:101", "Bob");

    std::string name = redis.GET("user:100");
    std::cout << "User 100: " << name << std::endl;  // Output: User 100: Alice

    redis.DEL("user:101");
    std::string deleted = redis.GET("user:101");
    std::cout << "User 101: " << deleted << std::endl;  // Output: User 101: (nil)

    return 0;
}
```

---

## Build & Run

### Requirements

- **C++17** or later
- **g++** (MinGW on Windows, or any POSIX-compliant compiler)
- **pthread** library (included with MinGW)

### Build Instructions (MSYS2 + MinGW)

```bash
# Clone the repository
git clone https://github.com/yourusername/Redis-Lite.git
cd Redis-Lite

# Compile
g++ -std=c++17 -pthread -o redislite main.cpp RedisLite.cpp

# Run
./redislite
```

### Expected Output (Basic Test)

```
SET user:100 → Alice
SET user:101 → Bob
GET user:100 → Alice
DEL user:101
GET user:101 → (nil)
```

---

## Stress Testing

### Test Setup

- **10,000 concurrent `SET` requests** from multiple threads
- Each thread submits commands to the shared queue
- Worker processes all 10,000 commands serially

### Sample Output

```
Starting stress test...
Spawning 100 threads (each sending 100 SET commands)...
✓ All threads finished enqueueing commands
✓ Worker processed 10,000 commands
Total time: 697 ms
Throughput: ~14,348 ops/sec
```

### Key Observations

- **Lock contention is minimal** because the worker thread doesn't hold locks while executing commands
- **Queue operations are fast** (`push` and `pop` are O(1) with `std::queue`)
- **Single-threaded execution eliminates** data races on the hash map

---

## Learning Outcomes

Building Redis-Lite taught me:

1. **Why Redis is single-threaded**: Lock-free command execution is faster than coordinating multiple threads on a shared data structure
2. **Producer–consumer patterns**: How to synchronize multiple producers with a single consumer using `std::condition_variable`
3. **Promise/future idiom**: A clean way to return results from asynchronous operations
4. **Performance trade-offs**: Single-threaded execution is a bottleneck for CPU-heavy operations, but perfect for memory-bound workloads
5. **C++ concurrency primitives**: Practical experience with `std::mutex`, `std::unique_lock`, `std::promise`, and `std::future`

---

## Future Improvements

This project is a foundation for more advanced features:

- [ ] **Networking**: Add TCP server using `asio` or raw sockets to accept remote clients
- [ ] **Persistence**: Implement RDB snapshots or AOF (append-only file) logging
- [ ] **Data Structures**: Support lists, sets, sorted sets (like real Redis)
- [ ] **Expiration**: Add TTL (time-to-live) for keys
- [ ] **Pipelining**: Batch multiple commands in a single request
- [ ] **Lua Scripting**: Embed LuaJIT for atomic multi-command operations
- [ ] **Replication**: Master-slave architecture for high availability

---



---

## License

MIT License – See [LICENSE](LICENSE) for details.

---

## Author

Built with ☕ and curiosity by [Amit Kumar](https://github.com/amitkumar2308)  
*"The best way to understand systems is to build them from scratch."*
