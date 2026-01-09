#include <iostream>
#include <thread>
#include "RedisLite.h"

void threadFuncSet(RedisLite& redis, const std::string& key, const std::string& value) {
    redis.set(key, value);
}

void threadFuncGet(RedisLite& redis, const std::string& key) {
    std::cout << key << ": " << redis.get(key) << std::endl;
}

int main() {
    RedisLite redis;

    std::thread t1(threadFuncSet, std::ref(redis), "user1", "amit");
    std::thread t2(threadFuncSet, std::ref(redis), "user2", "sanjay");

    t1.join();
    t2.join();

    std::thread t3(threadFuncGet, std::ref(redis), "user1");
    std::thread t4(threadFuncGet, std::ref(redis), "user2");

    t3.join();
    t4.join();

    return 0;
}
