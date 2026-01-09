#include "RedisLite.h"

RedisLite::RedisLite() {
    worker = std::thread(&RedisLite::workerLoop, this);
}

RedisLite::~RedisLite() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = true;
    }
    cv.notify_one();
    worker.join();
}

void RedisLite::workerLoop() {
    while (true) {
        Command cmd;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [&]() {
                return stop || !commandQueue.empty();
            });

            if (stop && commandQueue.empty())
                break;

            cmd = std::move(commandQueue.front());
            commandQueue.pop();
        }

        auto now = std::chrono::steady_clock::now();

        switch (cmd.type) {

        case CommandType::SET: {
            ValueEntry entry;
            entry.value = cmd.value;
            entry.hasTTL = false;
            store[cmd.key] = entry;
            break;
        }

        case CommandType::SET_TTL: {
            ValueEntry entry;
            entry.value = cmd.value;
            entry.hasTTL = true;
            entry.expireAt = now + std::chrono::seconds(cmd.ttlSeconds);
            store[cmd.key] = entry;
            break;
        }

        case CommandType::GET: {
            if (!store.count(cmd.key)) {
                cmd.result.set_value("");
                break;
            }

            auto &entry = store[cmd.key];

            if (entry.hasTTL && now >= entry.expireAt) {
                store.erase(cmd.key); // expire
                cmd.result.set_value("");
            } else {
                cmd.result.set_value(entry.value);
            }
            break;
        }

        case CommandType::DEL:
            store.erase(cmd.key);
            break;
        }
    }
}

void RedisLite::set(const std::string& key, const std::string& value) {
    Command cmd;
    cmd.type = CommandType::SET;
    cmd.key = key;
    cmd.value = value;

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(std::move(cmd));
    }
    cv.notify_one();
}

void RedisLite::setWithTTL(const std::string& key,
                           const std::string& value,
                           int ttlSeconds) {
    Command cmd;
    cmd.type = CommandType::SET_TTL;
    cmd.key = key;
    cmd.value = value;
    cmd.ttlSeconds = ttlSeconds;

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(std::move(cmd));
    }
    cv.notify_one();
}

std::string RedisLite::get(const std::string& key) {
    Command cmd;
    cmd.type = CommandType::GET;
    cmd.key = key;

    auto future = cmd.result.get_future();

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(std::move(cmd));
    }
    cv.notify_one();

    return future.get();
}

void RedisLite::del(const std::string& key) {
    Command cmd;
    cmd.type = CommandType::DEL;
    cmd.key = key;

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(std::move(cmd));
    }
    cv.notify_one();
}
