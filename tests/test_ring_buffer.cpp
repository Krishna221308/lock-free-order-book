#include "lob/ring_buffer.hpp"
#include <cassert>
#include <iostream>
#include <thread>

using lob::RingBuffer;

void test_basic_push_pop() {
    RingBuffer<int, 8> rb;

    assert(rb.empty());

    rb.push(10);
    rb.push(20);
    rb.push(30);

    auto a = rb.pop();
    auto b = rb.pop();
    auto c = rb.pop();
    auto d = rb.pop();

    assert(a.has_value() && *a == 10);
    assert(b.has_value() && *b == 20);
    assert(c.has_value() && *c == 30);
    assert(!d.has_value());
    assert(rb.empty());

    std::cout << "test_basic_push_pop passed!\n";
}

void test_full_buffer() {
    RingBuffer<int, 4> rb;

    assert(rb.push(1));
    assert(rb.push(2));
    assert(rb.push(3));
    assert(rb.push(4));
    assert(!rb.push(5)); // full — must return false

    assert(*rb.pop() == 1);
    assert(*rb.pop() == 2);
    assert(*rb.pop() == 3);
    assert(*rb.pop() == 4);
    assert(!rb.pop().has_value());

    std::cout << "test_full_buffer passed!\n";
}

void test_concurrent_sum_10x() {
    constexpr int N = 1000000;
    const long long EXPECTED = static_cast<long long>(N) * (N + 1) / 2;
    constexpr int RUNS = 10;

    using RB = RingBuffer<int, 1 << 17>;

    for (int run = 1; run <= RUNS; ++run) {
        RB rb;
        long long sum = 0;

        std::thread producer([&rb, N]() {
            for (int i = 1; i <= N; ++i) {
                while (!rb.push(i)) {
                    std::this_thread::yield();
                }
            }
        });

        std::thread consumer([&rb, &sum, N]() {
            int received = 0;
            while (received < N) {
                auto v = rb.pop();
                if (v) {
                    sum += *v;
                    ++received;
                }
            }
        });

        producer.join();
        consumer.join();

        assert(sum == EXPECTED);
        std::cout << "  run " << run << "/" << RUNS << "  sum=" << sum << "  OK\n";
    }

    std::cout << "test_concurrent_sum_10x passed! (10/10 identical sums)\n";
}

void test_emplace_no_copy() {
    // Part A: basic emplace round-trip
    {
        RingBuffer<int, 8> rb;
        rb.emplace(42);
        auto v = rb.pop();
        assert(v.has_value() && *v == 42);
    }

    // Part B: copy count — write-side copies only (pop() always copies once on read)
    struct TrivialCounter {
        int value = 0;
        int* copy_hits = nullptr;

        TrivialCounter() = default;
        explicit TrivialCounter(int v, int* hits) : value(v), copy_hits(hits) {}

        TrivialCounter(const TrivialCounter& o)
            : value(o.value), copy_hits(o.copy_hits) {
            if (copy_hits) (*copy_hits)++;
        }
        TrivialCounter(TrivialCounter&&) = default;
        ~TrivialCounter() = default;
    };

    static_assert(std::is_trivially_destructible_v<TrivialCounter>);

    // push() — 1 write-side copy
    {
        int copies = 0;
        RingBuffer<TrivialCounter, 8> rb;

        TrivialCounter src(99, &copies);
        rb.push(src);
        int write_copies = copies;

        auto v = rb.pop();
        assert(v.has_value() && v->value == 99);
        assert(write_copies >= 1);
        std::cout << "  push()    write_copies=" << write_copies << " (expected >= 1)\n";
    }

    // emplace() — 0 write-side copies
    {
        int copies = 0;
        RingBuffer<TrivialCounter, 8> rb;

        rb.emplace(99, &copies);
        int write_copies = copies;

        auto v = rb.pop();
        assert(v.has_value() && v->value == 99);
        assert(write_copies == 0);
        std::cout << "  emplace() write_copies=" << write_copies << " (expected 0)\n";
    }

    std::cout << "test_emplace_no_copy passed!\n";
}

int main() {
    test_basic_push_pop();
    test_full_buffer();
    test_concurrent_sum_10x();
    test_emplace_no_copy();

    std::cout << "\nALL STAGE 4 RING BUFFER TESTS PASSED!\n";
    return 0;
}
