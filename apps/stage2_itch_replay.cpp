#include "lob/order_book.hpp"
#include "lob/itch_parser.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    // Default to the shared fixture file, but allow overriding via command line
    std::string filepath = "data/sample.itch";
    if (argc > 1) {
        filepath = argv[1];
    }

    lob::OrderBook book;
    lob::ItchParser parser;
    
    std::cout << "Starting ITCH replay of " << filepath << "...\n";
    
    // This runs the entire file through your switch cases
    parser.parse_file(filepath, book);
    
    std::cout << "Replay complete!\n\n";

    // Print final state to verify correctness
    auto best_bid = book.best_bid();
    auto best_ask = book.best_ask();

    std::cout << "--- Final Book State ---\n";
    if (best_bid) {
        std::cout << "Best Bid: " << *best_bid << "\n";
    } else {
        std::cout << "Best Bid: [Empty]\n";
    }
    
    if (best_ask) {
        std::cout << "Best Ask: " << *best_ask << "\n";
    } else {
        std::cout << "Best Ask: [Empty]\n";
    }

    return 0;
}