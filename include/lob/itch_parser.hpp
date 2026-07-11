#pragma once
#include "lob/itch_messages.hpp"
#include "lob/order_book.hpp"
#include <iostream>
#include <cstdint>
#include <string>

namespace lob {
    class ItchParser {
        private:
            
            inline uint16_t to_uint16_be(const uint8_t* buffer) {
                return (static_cast<uint16_t>(buffer[0] << 8) | static_cast<uint16_t> (buffer[1]));
            }

            inline uint32_t to_uint32_be(const uint8_t* buffer) {
                return (static_cast<uint32_t>(buffer[0] << 24) | static_cast<uint32_t>(buffer[1] << 16) | static_cast<uint32_t>(buffer[2] << 8) | static_cast<uint32_t>(buffer[3]));
            }

            // For timestamp
            inline uint64_t to_uint48_be(const uint8_t* buffer) {
                return (static_cast<uint64_t>(buffer[0] << 40) | static_cast<uint64_t>(buffer[1] << 32) | static_cast<uint64_t>(buffer[2] << 24) | static_cast<uint64_t>(buffer[3] << 16) | static_cast<uint64_t>(buffer[4] << 8) | static_cast<uint64_t>(buffer[5]));
            }

            inline uint64_t to_uint64_be(const uint8_t* buffer) {
                return (static_cast<uint64_t>(buffer[0] << 56) | static_cast<uint64_t>(buffer[1] << 48) | static_cast<uint64_t>(buffer[2] << 40) | static_cast<uint64_t>(buffer[3] << 32) | static_cast<uint64_t>(buffer[4] << 24) | static_cast<uint64_t>(buffer[5] << 16) | static_cast<uint64_t>(buffer[6] << 8) | static_cast<uint64_t>(buffer[7]));
            }

        public:
            void parse_file(const std::string& filepath, OrderBook& book);
    }
}