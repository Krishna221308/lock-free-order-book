#include "lob/itch_parser.hpp"
#include "lob/order_book.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdio> // for std::remove

// Helper to write big-endian integers for our test fixture
void write_uint16_be(std::ofstream& out, uint16_t val) {
    uint8_t bytes[2] = {
        static_cast<uint8_t>((val >> 8) & 0xFF),
        static_cast<uint8_t>(val & 0xFF)
    };
    out.write(reinterpret_cast<const char*>(bytes), 2);
}

void write_uint32_be(std::ofstream& out, uint32_t val) {
    uint8_t bytes[4] = {
        static_cast<uint8_t>((val >> 24) & 0xFF),
        static_cast<uint8_t>((val >> 16) & 0xFF),
        static_cast<uint8_t>((val >> 8) & 0xFF),
        static_cast<uint8_t>(val & 0xFF)
    };
    out.write(reinterpret_cast<const char*>(bytes), 4);
}

void write_uint48_be(std::ofstream& out, uint64_t val) {
    uint8_t bytes[6] = {
        static_cast<uint8_t>((val >> 40) & 0xFF),
        static_cast<uint8_t>((val >> 32) & 0xFF),
        static_cast<uint8_t>((val >> 24) & 0xFF),
        static_cast<uint8_t>((val >> 16) & 0xFF),
        static_cast<uint8_t>((val >> 8) & 0xFF),
        static_cast<uint8_t>(val & 0xFF)
    };
    out.write(reinterpret_cast<const char*>(bytes), 6);
}

void write_uint64_be(std::ofstream& out, uint64_t val) {
    uint8_t bytes[8] = {
        static_cast<uint8_t>((val >> 56) & 0xFF),
        static_cast<uint8_t>((val >> 48) & 0xFF),
        static_cast<uint8_t>((val >> 40) & 0xFF),
        static_cast<uint8_t>((val >> 32) & 0xFF),
        static_cast<uint8_t>((val >> 24) & 0xFF),
        static_cast<uint8_t>((val >> 16) & 0xFF),
        static_cast<uint8_t>((val >> 8) & 0xFF),
        static_cast<uint8_t>(val & 0xFF)
    };
    out.write(reinterpret_cast<const char*>(bytes), 8);
}

void create_test_itch_file(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    
    // Add Order Message ('A')
    out.put('A');
    write_uint16_be(out, 1); // stock_locate
    write_uint16_be(out, 1); // tracking_number
    write_uint48_be(out, 123456789); // timestamp
    write_uint64_be(out, 1001); // order_reference_id
    out.put('B'); // buy_sell_indicator (Buy)
    write_uint32_be(out, 100); // shares
    out.write("AAPL    ", 8); // stock
    write_uint32_be(out, 1500000); // price (150.0000)

    out.close();
}

int main() {
    std::cout << "Running ITCH parser tests...\n";

    lob::ItchParser parser;
    
    // Test 1: Message size lookups
    assert(parser.get_message_size('A') == 36);
    assert(parser.get_message_size('E') == 31);
    assert(parser.get_message_size('D') == 19);
    assert(parser.get_message_size('S') == 12);
    std::cout << "  [OK] Message sizes correct.\n";

    // Test 2: Parsing binary data
    std::string test_file = "test_fixture.itch";
    create_test_itch_file(test_file);

    lob::OrderBook book;
    parser.parse_file(test_file, book);

    // Verify the book state after parsing
    auto best_bid = book.best_bid();
    assert(best_bid.has_value());
    assert(best_bid.value() == 1500000); // We wrote 1500000 as the price
    std::cout << "  [OK] Binary file parsed and routed to OrderBook.\n";

    // Cleanup
    std::remove(test_file.c_str());

    std::cout << "All tests passed!\n";
    return 0;
}