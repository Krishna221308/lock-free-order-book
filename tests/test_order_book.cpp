#include "lob/order_book.hpp"
#include <cassert>
#include <iostream>

using namespace lob;

void test_empty_book_add() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    assert(book.best_bid() == 100);
    assert(!book.best_ask().has_value());
    std::cout << "test_empty_book_add passed!\n";
}
void test_cancel_non_existent() {
    OrderBook book;
    book.cancel_order(999); // Should gracefully do nothing and not crash
    assert(!book.best_bid().has_value());
    std::cout << "test_cancel_non_existent passed!\n";
}
void test_exact_price_cross() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.add_order({2, Side::Sell, 100, 10, 2});
    book.match();
    // Both should be fully filled and removed
    assert(!book.best_bid().has_value());
    assert(!book.best_ask().has_value());
    std::cout << "test_exact_price_cross passed!\n";
}
void test_partial_fill() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.add_order({2, Side::Sell, 99, 4, 2}); // Sells 4 at 99
    book.match();
    
    // Sell order is fully filled and gone. Buy order has 6 left at 100.
    assert(book.best_bid() == 100);
    assert(!book.best_ask().has_value());
    std::cout << "test_partial_fill passed!\n";
}
void test_multi_fill() {
    OrderBook book;
    book.add_order({1, Side::Sell, 101, 5, 1});
    book.add_order({2, Side::Sell, 102, 5, 2});
    
    // Sweeps the book, buying 10 total across both asks
    book.add_order({3, Side::Buy, 105, 10, 3}); 
    book.match();
    
    assert(!book.best_bid().has_value()); // Buy order is fully filled
    assert(!book.best_ask().has_value()); // Both sell orders are fully filled
    std::cout << "test_multi_fill passed!\n";
}
void test_cancel_then_readd() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.cancel_order(1);
    assert(!book.best_bid().has_value()); // Should be empty
    
    book.add_order({1, Side::Buy, 100, 10, 2}); // Re-add with same ID
    assert(book.best_bid() == 100);
    std::cout << "test_cancel_then_readd passed!\n";
}
void test_modify_order() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    
    // Edge case: modifying to 0 should cleanly route to your cancel logic
    book.modify_order(1, 0); 
    assert(!book.best_bid().has_value());
    
    // Normal case: modifying quantity down
    book.add_order({2, Side::Sell, 102, 10, 2});
    book.modify_order(2, 5); // Reduce to 5
    
    // Cross it exactly to prove the quantity was actually reduced
    book.add_order({3, Side::Buy, 105, 5, 3});
    book.match();
    
    assert(!book.best_ask().has_value()); // If it was 5, it should now be 0 and removed
    
    std::cout << "test_modify_order passed!\n";
}
int main() {
    test_empty_book_add();
    test_cancel_non_existent();
    test_exact_price_cross();
    test_partial_fill();
    test_multi_fill();
    test_cancel_then_readd();
    test_modify_order();
    
    std::cout << "\nALL STAGE 1 TESTS PASSED!\n";
    return 0;
}