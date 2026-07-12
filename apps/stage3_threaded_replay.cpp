#include <iostream>
#include <thread>

#include "lob/itch_parser.hpp"
#include "lob/mutex_queue.hpp"
#include "lob/order_book.hpp"

using namespace lob;

int main() {
    MutexQueue<ParsedMessage> queue;
    OrderBook book;

    // Thread A: parse only, push each message; push one END_OF_FILE
    // sentinel when the parser is exhausted.
    std::thread producer([&queue]() {
        ItchParser parser("data/sample.itch");
        ParsedMessage msg;
        while (parser.next(msg)) {
            queue.push(msg);
        }
        ParsedMessage eof;
        eof.type = MessageType::END_OF_FILE;
        queue.push(eof);
    });

    // Thread B: apply only, stop on the sentinel.
    std::thread consumer([&queue, &book]() {
        while (true) {
            ParsedMessage msg = queue.pop();
            if (msg.type == MessageType::END_OF_FILE) {
                break;
            }
            apply_to_book(msg, book);
        }
    });

    producer.join();
    consumer.join();

    std::cout << "Final book state (Stage 3, threaded):\n";
    std::cout << "Best bid: " << book.best_bid().value_or(-1) << "\n";
    std::cout << "Best ask: " << book.best_ask().value_or(-1) << "\n";

    return 0;
}