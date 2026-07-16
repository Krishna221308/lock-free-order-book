#include "lob/mutex_queue.hpp"
#include "lob/ring_buffer.hpp"
#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

using lob::MutexQueue;
using lob::RingBuffer;

using Ns = std::chrono::nanoseconds;
using Clock = std::chrono::high_resolution_clock;

static constexpr int ROUNDS = 100000;

static long long percentile(std::vector<long long>& v, double p) {
    int idx = static_cast<int>(v.size() * p / 100.0);
    if (idx >= static_cast<int>(v.size())) idx = static_cast<int>(v.size()) - 1;
    return v[idx];
}

void bench_mutex_queue() {
    MutexQueue<int> q;
    std::vector<long long> latencies;
    latencies.reserve(ROUNDS);

    std::thread producer([&q]() {
        for (int i = 0; i < ROUNDS; ++i) {
            q.push(i);
        }
    });

    while (auto val = q.pop()) {
        auto start = Clock::now();
        (void)val;
        auto end = Clock::now();
        latencies.push_back(std::chrono::duration_cast<Ns>(end - start).count());
    }

    producer.join();

    std::sort(latencies.begin(), latencies.end());
    std::cout << "MutexQueue (" << ROUNDS << " rounds):\n";
    std::cout << "  p50  = " << percentile(latencies, 50)  << " ns\n";
    std::cout << "  p99  = " << percentile(latencies, 99)  << " ns\n";
    std::cout << "  p99.9= " << percentile(latencies, 99.9) << " ns\n";
}

void bench_ring_buffer() {
    using RB = RingBuffer<int, 1 << 17>;
    auto rb = std::make_unique<RB>();
    std::vector<long long> latencies;
    latencies.reserve(ROUNDS);

    std::thread producer([&rb]() {
        for (int i = 0; i < ROUNDS; ++i) {
            while (!rb->push(i)) {
                std::this_thread::yield();
            }
        }
    });

    int recieved = 0;
    while (recieved < ROUNDS) {
        auto start = Clock::now();
        auto val = rb->pop();
        auto end = Clock::now() ;
        if (val) {
            latencies.push_back(std::chrono::duration_cast<Ns>(end - start).count());    
            ++recieved;
        }    
    }

    producer.join();

    std::sort(latencies.begin(), latencies.end());
    std::cout << "RingBuffer (" << ROUNDS << " rounds):\n";
    std::cout << "  p50  = " << percentile(latencies, 50)  << " ns\n";
    std::cout << "  p99  = " << percentile(latencies, 99)  << " ns\n";
    std::cout << "  p99.9= " << percentile(latencies, 99.9) << " ns\n";
}

int main() {
    bench_mutex_queue();
    std::cout<<"\n";
    bench_ring_buffer();
    return 0;
}