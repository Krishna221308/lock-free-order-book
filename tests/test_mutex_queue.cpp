#include "lob/mutex_queue.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <numeric>
#include <cstdlib>

using lob::MutexQueue;

void test_basic_fifo_order() {
    MutexQueue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);

    auto a = q.pop();
    auto b = q.pop();
    auto c = q.pop();

    assert(a.has_value() && *a == 1);
    assert(b.has_value() && *b == 2);
    assert(c.has_value() && *c == 3);
    std::cout << "test_basic_fifo_order passed!\n";
}

void test_pop_blocks_until_pushed() {
    MutexQueue<int> q;
    auto start = std::chrono::steady_clock::now();

    std::thread producer([&q]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        q.push(42);
    });

    auto result = q.pop(); // must block ~200ms, not return early/garbage
    auto elapsed = std::chrono::steady_clock::now() - start;

    producer.join();
    assert(result.has_value() && *result == 42);
    assert(elapsed >= std::chrono::milliseconds(180)); // small slack for scheduling
    std::cout << "test_pop_blocks_until_pushed passed!\n";
}

void test_concurrent_sum_1000_with_random_delays() {
    MutexQueue<int> q;
    const int N = 1000;
    long long expected_sum = 0;
    for (int i = 1; i <= N; ++i) expected_sum += i;

    std::thread producer([&q, N]() {
        for (int i = 1; i <= N; ++i) {
            if (i % 50 == 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(rand() % 500));
            }
            q.push(i);
        }
        q.close();
    });

    long long actual_sum = 0;
    while (auto item = q.pop()) {
        actual_sum += *item;
    }

    producer.join();
    assert(actual_sum == expected_sum);
    std::cout << "test_concurrent_sum_1000_with_random_delays passed! (sum=" << actual_sum << ")\n";
}

void test_close_unblocks_waiting_consumer() {
    MutexQueue<int> q;

    std::thread closer([&q]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        q.close(); // consumer must wake up from pop() because of this, not a push
    });

    auto result = q.pop(); // queue is empty and will stay empty
    closer.join();

    assert(!result.has_value()); // nullopt == "producer is done, nothing more coming"
    std::cout << "test_close_unblocks_waiting_consumer passed!\n";
}

int main() {
    test_basic_fifo_order();
    test_pop_blocks_until_pushed();
    test_concurrent_sum_1000_with_random_delays();
    test_close_unblocks_waiting_consumer();

    std::cout << "\nALL STAGE 3 QUEUE TESTS PASSED!\n";
    return 0;
}