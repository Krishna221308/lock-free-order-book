#include "lob/order_book.hpp"
#include "lob/order_book_intrusive.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <new>
#include <vector>

// ---- Global allocation counter ----
static size_t g_alloc_count = 0;

void* operator new(size_t size) {
    g_alloc_count++;
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void operator delete(void* p) noexcept {
    std::free(p);
}

void operator delete(void* p, size_t) noexcept {
    std::free(p);
}

// ---- Helpers ----
using Clock = std::chrono::high_resolution_clock;
using Ms    = std::chrono::milliseconds;

static std::vector<lob::Order> generate_orders(int count, bool crossing) {
    std::vector<lob::Order> orders;
    orders.reserve(count);
    for (int i = 0; i < count; ++i) {
        // Alternate buy/sell; spread prices across 100 levels
        lob::Side side = (i % 2 == 0) ? lob::Side::Buy : lob::Side::Sell;
        int64_t price;
        if (crossing) {
            // Buys at 200+, sells at 100+ → guaranteed crosses
            price = (side == lob::Side::Buy) ? 200 + (i % 100)
                                             : 100 + (i % 100);
        } else {
            // Buys at 100+, sells at 200+ → no crosses
            price = (side == lob::Side::Buy) ? 100 + (i % 100)
                                             : 200 + (i % 100);
        }
        orders.push_back({static_cast<uint64_t>(i + 1), side, price, 10,
                          static_cast<uint64_t>(i + 1)});
    }
    return orders;
}

// ---- Templated benchmark runner ----
template <typename Book>
struct BenchResult {
    long long add_ms;
    long long cancel_ms;
    long long match_ms;
    size_t    add_allocs;
    size_t    cancel_allocs;
    size_t    match_allocs;
    size_t    trades_generated;
};

template <typename Book>
BenchResult<Book> run_bench(const std::vector<lob::Order>& non_crossing,
                            const std::vector<lob::Order>& crossing) {
    BenchResult<Book> result{};

    // --- 1. Add workload (non-crossing so nothing matches) ---
    {
        Book book;
        g_alloc_count = 0;
        auto start = Clock::now();
        for (const auto& o : non_crossing) book.add_order(o);
        auto end = Clock::now();
        result.add_ms     = std::chrono::duration_cast<Ms>(end - start).count();
        result.add_allocs = g_alloc_count;

        // --- 2. Cancel workload (cancel everything we just added) ---
        g_alloc_count = 0;
        start = Clock::now();
        for (const auto& o : non_crossing) book.cancel_order(o.id);
        end = Clock::now();
        result.cancel_ms     = std::chrono::duration_cast<Ms>(end - start).count();
        result.cancel_allocs = g_alloc_count;
    }

    // --- 3. Match workload (crossing orders) ---
    {
        Book book;
        for (const auto& o : crossing) book.add_order(o);

        g_alloc_count = 0;
        auto start = Clock::now();
        auto trades = book.match();
        auto end = Clock::now();
        result.match_ms        = std::chrono::duration_cast<Ms>(end - start).count();
        result.match_allocs    = g_alloc_count;
        result.trades_generated = trades.size();
    }

    return result;
}

template <typename Book>
void print_result(const char* name, const BenchResult<Book>& r) {
    std::cout << "  Add:    " << r.add_ms     << " ms  (" << r.add_allocs     << " allocs)\n";
    std::cout << "  Cancel: " << r.cancel_ms  << " ms  (" << r.cancel_allocs  << " allocs)\n";
    std::cout << "  Match:  " << r.match_ms   << " ms  (" << r.match_allocs   << " allocs)  → "
              << r.trades_generated << " trades\n";
}

// ---- Main ----
int main() {
    constexpr int N = 1'000'000;

    std::cout << "Generating " << N << " orders across 100 price levels...\n\n";

    auto non_crossing = generate_orders(N, false);
    auto crossing     = generate_orders(N, true);

    std::cout << "=== STL OrderBook ===\n";
    auto stl_result = run_bench<lob::OrderBook>(non_crossing, crossing);
    print_result("STL", stl_result);

    std::cout << "\n=== Intrusive OrderBook ===\n";
    auto intrusive_result = run_bench<lob::IntrusiveOrderBook>(non_crossing, crossing);
    print_result("Intrusive", intrusive_result);

    std::cout << "\n=== Comparison ===\n";
    std::cout << "  Add speedup:    " 
              << (stl_result.add_ms > 0 ? static_cast<double>(stl_result.add_ms) / intrusive_result.add_ms : 0)
              << "x\n";
    std::cout << "  Cancel speedup: " 
              << (stl_result.cancel_ms > 0 ? static_cast<double>(stl_result.cancel_ms) / intrusive_result.cancel_ms : 0)
              << "x\n";
    std::cout << "  Match speedup:  " 
              << (stl_result.match_ms > 0 ? static_cast<double>(stl_result.match_ms) / intrusive_result.match_ms : 0)
              << "x\n";
    std::cout << "  Add alloc reduction:    " << stl_result.add_allocs << " → " << intrusive_result.add_allocs << "\n";
    std::cout << "  Cancel alloc reduction: " << stl_result.cancel_allocs << " → " << intrusive_result.cancel_allocs << "\n";
    std::cout << "  Match alloc reduction:  " << stl_result.match_allocs << " → " << intrusive_result.match_allocs << "\n";

    return 0;
}
