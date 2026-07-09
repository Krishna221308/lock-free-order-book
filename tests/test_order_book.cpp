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
    book.cancel_order(999); 
    assert(!book.best_bid().has_value());
    std::cout << "test_cancel_non_existent passed!\n";
}

void test_exact_price_cross() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.add_order({2, Side::Sell, 100, 10, 2});
    auto trades = book.match();
    
    assert(trades.size() == 1);
    assert(trades[0].quantity == 10);
    assert(!book.best_bid().has_value());
    assert(!book.best_ask().has_value());
    std::cout << "test_exact_price_cross passed!\n";
}

void test_partial_fill() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.add_order({2, Side::Sell, 99, 4, 2}); 
    auto trades = book.match();
    
    assert(trades.size() == 1);
    assert(trades[0].quantity == 4);
    assert(trades[0].price == 100); // Resting order sets the price
    assert(book.best_bid() == 100);
    assert(!book.best_ask().has_value());
    std::cout << "test_partial_fill passed!\n";
}

void test_multi_fill() {
    OrderBook book;
    book.add_order({1, Side::Sell, 101, 5, 1});
    book.add_order({2, Side::Sell, 102, 5, 2});
    book.add_order({3, Side::Buy, 105, 10, 3}); 
    auto trades = book.match();
    
    assert(trades.size() == 2);
    assert(trades[0].quantity == 5);
    assert(trades[1].quantity == 5);
    assert(!book.best_bid().has_value()); 
    assert(!book.best_ask().has_value()); 
    std::cout << "test_multi_fill passed!\n";
}

void test_cancel_then_readd() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.cancel_order(1);
    assert(!book.best_bid().has_value()); 
    
    book.add_order({1, Side::Buy, 100, 10, 2}); 
    assert(book.best_bid() == 100);
    std::cout << "test_cancel_then_readd passed!\n";
}

void test_modify_order() {
    OrderBook book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.modify_order(1, 0); 
    assert(!book.best_bid().has_value());
    
    book.add_order({2, Side::Sell, 102, 10, 2});
    book.modify_order(2, 5); 
    
    book.add_order({3, Side::Buy, 105, 5, 3});
    auto trades = book.match();
    
    assert(trades.size() == 1);
    assert(!book.best_ask().has_value()); 
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
