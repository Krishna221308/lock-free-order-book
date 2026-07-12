#pragma once
#include <queue>
#include <optional>
#include <mutex>
#include <condition_variable>


namespace lob {
    template <typename T>
    class MutexQueue {
    private:
        std::queue<T> q;
        mutable std::mutex m;
        std::condition_variable cv_;
        bool closed_ = false;

    public:
        void push(T value) {
            std::lock_guard<std::mutex> lock(m);
            q.push(std::move(value));
            cv_.notify_one();
        }

        void close() {
            std::lock_guard<std::mutex> lock(m);
            closed_ = 1;
            cv_.notify_all();
        }

        std::optional<T> pop() {
            std::unique_lock<std::mutex> lock(m);
            cv_.wait(lock, [this]() { return !q.empty() || closed_; });

            if (q.empty()) return std::nullopt;

            T value = std::move(q.front());
            q.pop();
            return value;
        }
    };
}