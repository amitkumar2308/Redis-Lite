# Redis-Lite

A lightweight, thread-safe, in-memory key-value store implementation in C++ inspired by Redis. This project demonstrates concurrent programming concepts including mutex locks, thread synchronization, and safe data structure access.

## 🚀 Features

- **Thread-Safe Operations**: All operations (SET, GET, DEL) are protected with mutex locks
- **In-Memory Storage**: Fast key-value storage using `std::unordered_map`
- **Concurrent Access**: Multiple threads can safely access the store simultaneously
- **Simple API**: Clean and intuitive interface for basic Redis-like operations
- **Lightweight**: Minimal dependencies, uses only C++ Standard Library

## 📋 Architecture

### Core Components

#### RedisLite Class
The main class that provides the key-value store functionality:

```cpp
class RedisLite {
private:
    std::unordered_map<std::string, std::string> store;  // Key-value storage
    std::queue<Command> commandQueue;                     // Command queue (for future async processing)
    std::mutex queueMutex;                                // Queue synchronization
    std::condition_variable cv;                           // Condition variable for worker thread
    std::thread worker;                                   // Background worker thread
    bool stop;                                            // Stop flag for worker thread

public:
    void set(const std::string &key, const std::string &value);
    std::string get(const std::string &key);
    void del(const std::string &key);
};
```

#### Command Structure
Defines the command types for potential asynchronous processing:

```cpp
enum class CommandType {
    SET,
    GET,
    DEL
};

struct Command {
    CommandType type;
    std::string key;
    std::string value;
};
```

## 🔧 API Reference

### `void set(const std::string &key, const std::string &value)`
Stores a key-value pair in the database.
- **Thread-Safe**: Yes
- **Parameters**:
  - `key`: The key to store
  - `value`: The value to associate with the key
- **Returns**: void

### `std::string get(const std::string &key)`
Retrieves the value associated with a key.
- **Thread-Safe**: Yes
- **Parameters**:
  - `key`: The key to retrieve
- **Returns**: The value if found, empty string `" "` if not found

### `void del(const std::string &key)`
Deletes a key-value pair from the database.
- **Thread-Safe**: Yes
- **Parameters**:
  - `key`: The key to delete
- **Returns**: void

## 💻 Usage Example

```cpp
#include "RedisLite.h"
#include <thread>
#include <iostream>

void threadFuncSet(RedisLite &redis, const std::string &key, const std::string &value) {
    redis.set(key, value);
}

void threadFuncGet(RedisLite &redis, const std::string &key) {
    std::cout << key << ":" << redis.get(key) << std::endl;
}

int main() {
    RedisLite redis;

    // Create multiple threads to set values
    std::thread t1(threadFuncSet, std::ref(redis), "user1", "amit");
    std::thread t2(threadFuncSet, std::ref(redis), "user2", "sanjay");
    
    t1.join();
    t2.join();

    // Retrieve values from multiple threads
    std::thread t3(threadFuncGet, std::ref(redis), "user1");
    std::thread t4(threadFuncGet, std::ref(redis), "user2");
    
    t3.join();
    t4.join();

    return 0;
}
```

## 🛠️ Compilation & Running

### Prerequisites
- C++11 or later
- GCC/Clang/MSVC compiler with C++11 support

### Compile
```bash
# Using g++
g++ -std=c++11 -pthread main.cpp RedisLite.cpp -o redis-lite

# Using clang++
clang++ -std=c++11 -pthread main.cpp RedisLite.cpp -o redis-lite

# On Windows with MSVC
cl /EHsc /std:c++11 main.cpp RedisLite.cpp /Fe:redis-lite.exe
```

### Run
```bash
# Linux/Mac
./redis-lite

# Windows
redis-lite.exe
```

## 🔒 Thread Safety

All public methods (`set`, `get`, `del`) use `std::lock_guard<std::mutex>` to ensure thread-safe access to the underlying data store. This prevents race conditions when multiple threads access the store concurrently.

**Note**: There's a minor inconsistency in the current implementation - the header file declares `queueMutex` but the implementation uses `mtx`. This should be aligned for consistency.

## 📁 Project Structure

```
Redis-Lite/
├── RedisLite.h       # Header file with class declaration
├── RedisLite.cpp     # Implementation of RedisLite methods
├── main.cpp          # Example usage and testing
└── README.md         # This file
```

## 🚧 Known Issues & Future Enhancements

### Current Issues
1. **Mutex naming inconsistency**: Header declares `queueMutex` but implementation uses `mtx`
2. **Worker thread not implemented**: The `workerLoop()` method and worker thread are declared but not implemented
3. **Command queue unused**: The command queue structure is defined but not utilized

### Planned Features
- [ ] Implement asynchronous command processing with worker thread
- [ ] Add TTL (Time To Live) support for keys
- [ ] Implement additional Redis commands (INCR, DECR, LPUSH, RPUSH, etc.)
- [ ] Add persistence (save to disk/load from disk)
- [ ] Implement pub/sub messaging
- [ ] Add command-line interface (CLI)
- [ ] Network interface support (TCP server)
- [ ] Benchmark suite for performance testing

## 📝 License

This is an educational project demonstrating concurrent programming concepts in C++.

## 👨‍💻 Author

Built as a learning project to understand:
- Thread synchronization in C++
- Mutex and lock guards
- Concurrent data structure access
- Redis-like key-value store design