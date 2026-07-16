#ifdef USE_INTRUSIVE
#include "lob/order_book_intrusive.hpp"
#else
#include "lob/order_book.hpp"
#endif
#include <cassert>
#include <iostream>

using namespace lob;

#ifdef USE_INTRUSIVE
using BookType = IntrusiveOrderBook;
#else
using BookType = OrderBook;
#endif

void test_empty_book_add() {
    BookType book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    assert(book.best_bid() == 100);
    assert(!book.best_ask().has_value());
    std::cout << "test_empty_book_add passed!\n";
}

void test_cancel_non_existent() {
    BookType book;
    book.cancel_order(999); 
    assert(!book.best_bid().has_value());
    std::cout << "test_cancel_non_existent passed!\n";
}

void test_exact_price_cross() {
    BookType book;
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
    BookType book;
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
    BookType book;
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
    BookType book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.cancel_order(1);
    assert(!book.best_bid().has_value()); 
    
    book.add_order({1, Side::Buy, 100, 10, 2}); 
    assert(book.best_bid() == 100);
    std::cout << "test_cancel_then_readd passed!\n";
}

void test_modify_order() {
    BookType book;
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

void test_duplicate_id_rejected() {
    BookType book;
    book.add_order({1, Side::Buy, 100, 10, 1});
    book.add_order({1, Side::Sell, 200, 5, 2}); // duplicate id, should be silently ignored
    
    assert(book.best_bid() == 100);
    assert(!book.best_ask().has_value()); // second order never entered
    std::cout << "test_duplicate_id_rejected passed!\n";
}

void test_modify_increase_loses_priority() {
    BookType book;
    book.add_order({1, Side::Sell, 100, 5, 1});  // first at price 100
    book.add_order({2, Side::Sell, 100, 5, 2});  // second at price 100
    
    book.modify_order(1, 10); // increase qty → should lose priority, go behind order 2
    
    book.add_order({3, Side::Buy, 100, 5, 3});   // buyer for 5 shares
    auto trades = book.match();
    
    assert(trades.size() == 1);
    assert(trades[0].sell_id == 2); // order 2 should fill first now
    std::cout << "test_modify_increase_loses_priority passed!\n";
}

void test_trade_price_by_timestamp() {
    BookType book;
    book.add_order({1, Side::Sell, 100, 10, 1}); // resting sell at 100 (earlier)
    book.add_order({2, Side::Buy, 105, 10, 2});  // aggressive buy at 105 (later)
    auto trades = book.match();
    
    assert(trades.size() == 1);
    assert(trades[0].price == 100); // sell was earlier, so trade at sell's price
    std::cout << "test_trade_price_by_timestamp passed!\n";
}

int main() {
    test_empty_book_add();
    test_cancel_non_existent();
    test_exact_price_cross();
    test_partial_fill();
    test_multi_fill();
    test_cancel_then_readd();
    test_modify_order();
    test_duplicate_id_rejected();
    test_modify_increase_loses_priority();
    test_trade_price_by_timestamp();
    
    std::cout << "\nALL STAGE 1 TESTS PASSED!\n";
    return 0;
}
