#include "lob/order_book.hpp"
#include <iostream>

using namespace lob;

void print_best(const OrderBook& book) {
    auto best_bid = book.best_bid();
    auto best_ask = book.best_ask();

    std::cout << "Book Bid: " << (best_bid ? std::to_string(*best_bid) : "None") << "|";
    std::cout << "Book Ask: " << (best_ask ? std::to_string(*best_ask) : "None") << "|";
}

int main() {
    OrderBook book;

    std::cout << "---Adding Orders---\n";
    // Add some bids (buyers willing to pay X)
    book.add_order({1, Side::Buy, 100, 10, 1000});
    book.add_order({2, Side::Buy, 99, 15, 1001});
    
    // Add some asks (sellers wanting Y)
    book.add_order({3, Side::Sell, 102, 5, 1002});
    book.add_order({4, Side::Sell, 105, 20, 1003});

    print_best(book);

    std::cout << "\n--- Adding a Crossing Order (Match should happen) ---\n";
    // Someone swoops in and sells at 99. This crosses with the bid at 100!
    book.add_order({5, Side::Sell, 99, 10, 1004});
    
    // Run the matching engine
    book.match();
    
    print_best(book); // Should print Best Bid: 99 | Best Ask: 102 (since the bid at 100 got filled and removed)
    return 0;
}