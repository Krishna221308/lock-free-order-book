#pragma once
#include "lob/itch_messages.hpp"
#include "lob/order_book.hpp"
#include <fstream>
#include <cstdint>
#include <string>

namespace lob {
    class ItchParser {
        private:
            std::ifstream file_;

            static uint16_t to_uint16_be(const uint8_t* buffer);
            static uint32_t to_uint32_be(const uint8_t* buffer);
            static uint64_t to_uint48_be(const uint8_t* buffer); // timestamp
            static uint64_t to_uint64_be(const uint8_t* buffer);

            bool read_add(AddOrderMessage& out);
            bool read_execute(ExecuteOrderMessage& out);
            bool read_delete(DeleteOrderMessage& out);

        public:
            ItchParser() = default;
            explicit ItchParser(const std::string& filepath);

            void open(const std::string& filepath);

            bool next(ParsedMessage& out);

            void parse_file(const std::string& filepath, OrderBook& order);
            int get_message_size(char type);
    };

    void apply_to_book(const ParsedMessage& out, OrderBook& book); // Apply one of the parsed messages to the book (change needed for threading).
}