#pragma once
#include <unordered_map>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include<chrono>

enum class CommandType {
    SET,
    GET,
    DEL,
    SET_TTL
};

struct ValueEntry{
   std::string value;
   bool hasTTL = false;
   std::chrono::steady_clock::time_point expireAt;

};

struct Command {
    CommandType type;
    std::string key;
    std::string value;
    int ttlSeconds = 0;
    std::promise<std::string> result; // Only used for GET
};

class RedisLite {
private:
    // Single-thread owned state (worker thread only)
    std::unordered_map<std::string, ValueEntry>store;

    // Producer-consumer queue
    std::queue<Command> commandQueue;
    std::mutex queueMutex;
    std::condition_variable cv;

    std::thread worker;
    bool stop = false;

    void workerLoop();

public:
    RedisLite();
    ~RedisLite();

    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    void del(const std::string& key);
    void setWithTTL(const std::string& key, const std::string& value, int ttlSeconds);
};
