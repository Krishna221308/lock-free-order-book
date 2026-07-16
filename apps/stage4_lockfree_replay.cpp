#include <iostream>
#include <thread>
#include <memory>

#include "lob/itch_parser.hpp"
#include "lob/order_book.hpp"
#include "lob/ring_buffer.hpp"

using namespace lob;

static constexpr std::size_t BUFFER_SIZE = 1 << 16;

static bool is_eof(const ParsedMessage& m) {
    return m.type == MessageType::END_OF_FILE;
}

int main() {
    auto rb = std::make_unique<RingBuffer<ParsedMessage, BUFFER_SIZE>>();
    OrderBook book;

    std::thread producer([&rb]() {
        ItchParser parser("data/sample.itch");
        ParsedMessage msg;

        while (parser.next(msg)) {
            while (!rb->emplace(msg)) {
                std::this_thread::yield();
            }
        }

        ParsedMessage eof_msg;
        eof_msg.type = MessageType::END_OF_FILE;
        while (!rb->emplace(eof_msg)) {
            std::this_thread::yield();
        }
    });

    std::thread consumer([&rb, &book]() {
        while (true) {
            auto msg = rb->pop();
            if (!msg) {
                std::this_thread::yield();
                continue;
            }
            if (is_eof(*msg)) break;
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
