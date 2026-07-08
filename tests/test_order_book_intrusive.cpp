#include "lob/order_book_intrusive.hpp"
#include <cassert>
#include <iostream>

int main() {
    lob::IntrusiveOrderBook book;

    // 1. Empty book
    assert(!book.best_bid().has_value());
    
    // 2. Add an order
    lob::Order o1{1, lob::Side::Buy, 100, 50, 1};
    book.add_order(o1);
    assert(book.best_bid() == 100);

    // 3. Add a crossing order
    lob::Order o2{2, lob::Side::Sell, 99, 50, 2};
    book.add_order(o2);
    book.match(); // They should fully fill and be removed

    // 4. Book is empty again
    assert(!book.best_bid().has_value());
    assert(!book.best_ask().has_value());
    
    std::cout << "IntrusiveOrderBook logic tests passed!\n";
    return 0;
}
