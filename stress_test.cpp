#include "RedisLite.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

int main() {
    RedisLite redis;

    const int THREADS = 10000;
    std::vector<std::thread> workers;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < THREADS; i++) {
        workers.emplace_back([&redis, i]() {
            redis.set("key" + std::to_string(i), "value");
        });
    }

    for (auto& t : workers) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Handled " << THREADS
              << " concurrent SET requests in "
              << duration.count() << " ms\n";

    return 0;
}
