#include "lob/order_book.hpp"
#include <iostream>

using namespace lob;

void print_best(const OrderBook& book) {
    auto best_bid = book.best_bid();
    auto best_ask = book.best_ask();

    std::cout << "Book Bid: " << (best_bid ? std::to_string(*best_bid) : "None") << " | ";
    std::cout << "Book Ask: " << (best_ask ? std::to_string(*best_ask) : "None") << "\n";
}

int main() {
    OrderBook book;

    std::cout << "--- Adding Orders ---\n";
    book.add_order({1, Side::Buy, 100, 10, 1000});
    book.add_order({2, Side::Buy, 99, 15, 1001});
    book.add_order({3, Side::Sell, 102, 5, 1002});
    book.add_order({4, Side::Sell, 105, 20, 1003});

    print_best(book);

    std::cout << "\n--- Adding a Crossing Order (Match should happen) ---\n";
    book.add_order({5, Side::Sell, 99, 10, 1004});
    
    // Run the matching engine and catch trades
    auto trades = book.match();
    
    std::cout << "\n--- Trades Executed ---\n";
    for (const auto& trade : trades) {
        std::cout << "Trade: Buy ID " << trade.buy_id 
                  << " matched Sell ID " << trade.sell_id 
                  << " for " << trade.quantity 
                  << " @ $" << trade.price << "\n";
    }

    std::cout << "\n--- Final State ---\n";
    print_best(book); 
    
    return 0;
}
