#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <optional>
#include <type_traits>

namespace lob {

template <typename T, std::size_t N>
class RingBuffer {
    static_assert(N >= 2 && (N & (N - 1)) == 0,
                  "RingBuffer size N must be a power of two >= 2");

    static_assert(std::is_trivially_destructible_v<T>,
                  "RingBuffer<T,N>: T must be trivially destructible");

    alignas(T) std::array<std::byte, N * sizeof(T)> storage_{};

    // head_ written by producer, tail_ written by consumer.
    // alignas(64) keeps them on separate cache lines to avoid false sharing.
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};

    T* slot(std::size_t idx) noexcept {
        return reinterpret_cast<T*>(storage_.data() + (idx & (N - 1)) * sizeof(T));
    }

public:
    bool push(const T& value) {
        std::size_t h = head_.load(std::memory_order_relaxed);
        std::size_t t = tail_.load(std::memory_order_acquire);

        if (h - t == N) return false;

        ::new (slot(h)) T(value);
        head_.store(h + 1, std::memory_order_release);
        return true;
    }

    template <typename... Args>
    bool emplace(Args&&... args) {
        std::size_t h = head_.load(std::memory_order_relaxed);
        std::size_t t = tail_.load(std::memory_order_acquire);

        if (h - t == N) return false;

        ::new (slot(h)) T(std::forward<Args>(args)...);
        head_.store(h + 1, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {
        std::size_t t = tail_.load(std::memory_order_relaxed);
        std::size_t h = head_.load(std::memory_order_acquire);

        if (t == h) return std::nullopt;

        T value = *slot(t);
        tail_.store(t + 1, std::memory_order_release);
        return value;
    }

    bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    static constexpr std::size_t capacity() noexcept { return N; }
};

}  // namespace lob
