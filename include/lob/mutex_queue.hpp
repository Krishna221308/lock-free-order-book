#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class MutexQueue {
private:
    std::queue<T> q;
    std::mutex m;
    std::condition_variable cv_;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(m);
        q.push(value);
        cv_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(m);
        cv_.wait(lock, [this]() { return !q.empty(); });

        T value = q.front();
        q.pop();
        return value;
    }
};