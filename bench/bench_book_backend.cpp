#include "lob/order_book.hpp"
#include "lob/order_book_intrusive.hpp"
#include <chrono>
#include <iostream>

int main() {
    const int NUM_ORDERS = 1000000;

    auto run_stl = []() {
        lob::OrderBook book;
        auto start = std::chrono::high_resolution_clock::now();
        for (uint64_t i = 1; i <= NUM_ORDERS; ++i) {
            book.add_order(lob::Order{i, lob::Side::Buy, 100, 10, i});
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    };

    auto run_intrusive = []() {
        lob::IntrusiveOrderBook book;
        auto start = std::chrono::high_resolution_clock::now();
        for (uint64_t i = 1; i <= NUM_ORDERS; ++i) {
            book.add_order(lob::Order{i, lob::Side::Buy, 100, 10, i});
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    };

    std::cout << "Inserting " << NUM_ORDERS << " orders...\n";
    std::cout << "STL Book Time:       " << run_stl() << " ms\n";
    std::cout << "Intrusive Book Time: " << run_intrusive() << " ms\n";

    return 0;
}
