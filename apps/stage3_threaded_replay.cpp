#include <iostream>
#include <thread>

#include "lob/itch_parser.hpp"
#include "lob/mutex_queue.hpp"
#include "lob/order_book.hpp"

using namespace lob;

int main() {
    MutexQueue<ParsedMessage> queue;
    OrderBook book;

    // Thread A: parse only, push each message. When the parser is
    // exhausted, close() the queue instead of pushing a sentinel —
    // that's the signal to the consumer that nothing more is coming.
    std::thread producer([&queue]() {
        ItchParser parser("data/sample.itch");
        ParsedMessage msg;
        while (parser.next(msg)) {
            queue.push(msg);
        }
        queue.close();
    });

    // Thread B: apply only, stop when pop() returns nullopt —
    // meaning the queue is both closed AND fully drained.
    std::thread consumer([&queue, &book]() {
        while (auto msg = queue.pop()) {
            apply_to_book(*msg, book);
        }
    });

    producer.join();
    consumer.join();
    
    std::cout << "Starting ITCH replay of data/sample.itch...\n";
    std::cout << "Replay complete!\n\n";

    std::cout << "--- Final Book State ---\n";
    auto best_bid = book.best_bid();
    auto best_ask = book.best_ask();

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