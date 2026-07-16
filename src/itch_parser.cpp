#include "lob/itch_parser.hpp"
#include <iostream>
#include <vector>
#include <cstring>

namespace lob {
    ItchParser::ItchParser(const std::string& filepath) {
        open(filepath);
    }

    void ItchParser::open(const std::string& filepath) {
        file_.open(filepath, std::ios::binary);
        if (!file_.is_open()) {
            std::cerr << "Failed to open ITCH file: " << filepath << "\n";
        }
    }

    uint16_t ItchParser::to_uint16_be(const uint8_t* buffer) {
        return (static_cast<uint16_t>(buffer[0]) << 8) |
               (static_cast<uint16_t>(buffer[1]));
    }
    uint32_t ItchParser::to_uint32_be(const uint8_t* buffer) {
        return (static_cast<uint32_t>(buffer[0]) << 24) |
               (static_cast<uint32_t>(buffer[1]) << 16) |
               (static_cast<uint32_t>(buffer[2]) << 8)  |
               (static_cast<uint32_t>(buffer[3]));
    }
    uint64_t ItchParser::to_uint48_be(const uint8_t* buffer) {
        return (static_cast<uint64_t>(buffer[0]) << 40) |
               (static_cast<uint64_t>(buffer[1]) << 32) |
               (static_cast<uint64_t>(buffer[2]) << 24) |
               (static_cast<uint64_t>(buffer[3]) << 16) |
               (static_cast<uint64_t>(buffer[4]) << 8)  |
               (static_cast<uint64_t>(buffer[5]));
    }
    uint64_t ItchParser::to_uint64_be(const uint8_t* buffer) {
        return (static_cast<uint64_t>(buffer[0]) << 56) |
               (static_cast<uint64_t>(buffer[1]) << 48) |
               (static_cast<uint64_t>(buffer[2]) << 40) |
               (static_cast<uint64_t>(buffer[3]) << 32) |
               (static_cast<uint64_t>(buffer[4]) << 24) |
               (static_cast<uint64_t>(buffer[5]) << 16) |
               (static_cast<uint64_t>(buffer[6]) << 8)  |
               (static_cast<uint64_t>(buffer[7]));
    }

    int ItchParser::get_message_size(char type) {
        switch (type) {
            case 'S': return 12; // System Event
            case 'R': return 39; // Stock Directory
            case 'H': return 25; // Stock Trading Action
            case 'Y': return 20; // Reg SHO Restriction
            case 'L': return 26; // Market Participant Position
            case 'V': return 35; // MWCB Decline Level
            case 'W': return 12; // MWCB Status
            case 'K': return 28; // IPO Quoting Period
            case 'J': return 35; // LULD Auction Collar
            case 'h': return 21; // Operational Halt
            case 'A': return 36; // Add Order
            case 'F': return 40; // Add Order with MPID
            case 'E': return 31; // Order Executed
            case 'C': return 36; // Order Executed with Price
            case 'X': return 23; // Order Cancel
            case 'D': return 19; // Order Delete
            case 'U': return 35; // Order Replace
            case 'P': return 44; // Trade
            case 'Q': return 40; // Cross Trade
            case 'B': return 19; // Broken Trade
            case 'I': return 50; // NOII
            case 'N': return 20; // Retail Interest
            default:  return 0;  // Unknown!
        }
    }

    bool ItchParser::read_add(AddOrderMessage& msg) {
        std::vector<uint8_t> buffer(35);
        if (!file_.read(reinterpret_cast<char*>(buffer.data()), 35)) return false;

        msg.stock_locate = to_uint16_be(buffer.data());
        msg.tracking_number = to_uint16_be(buffer.data() + 2);
        msg.timestamp = to_uint48_be(buffer.data() + 4);
        msg.order_reference_id = to_uint64_be(buffer.data() + 10);
        msg.buy_sell_indicator = static_cast<char>(buffer[18]);
        msg.shares = to_uint32_be(buffer.data() + 19);
        std::memcpy(msg.stock, buffer.data() + 23, 8);
        msg.price = to_uint32_be(buffer.data() + 31);
        return true;
    }

    bool ItchParser::read_execute(ExecuteOrderMessage& msg) {
        std::vector<uint8_t> buffer(30);
        if (!file_.read(reinterpret_cast<char*>(buffer.data()), 30)) return false;

        msg.stock_locate = to_uint16_be(buffer.data());
        msg.tracking_number = to_uint16_be(buffer.data() + 2);
        msg.timestamp = to_uint48_be(buffer.data() + 4);
        msg.order_reference_id = to_uint64_be(buffer.data() + 10);
        msg.executed_shares = to_uint32_be(buffer.data() + 18);
        msg.match_number = to_uint64_be(buffer.data() + 22);
        return true;
    }

    bool ItchParser::read_delete(DeleteOrderMessage& msg) {
        std::vector<uint8_t> buffer(18);
        if (!file_.read(reinterpret_cast<char*>(buffer.data()), 18)) return false;

        msg.stock_locate = to_uint16_be(buffer.data());
        msg.tracking_number = to_uint16_be(buffer.data() + 2);
        msg.timestamp = to_uint48_be(buffer.data() + 4);
        msg.order_reference_id = to_uint64_be(buffer.data() + 10);
        return true;
    }

    bool ItchParser::next(ParsedMessage& out) {
        char type_byte;
        while (file_.read(&type_byte, 1)) {
            switch (type_byte) {
                case 'A': {
                    if (!read_add(out.add)) break;
                    out.type = MessageType::ADD;
                    return 1;
                }
                case 'E' : {
                    if (!read_execute(out.execute)) break;
                    out.type = MessageType::EXECUTE;
                    return 1;
                }
                case 'D' : {
                    if (!read_delete(out.del)) break;
                    out.type = MessageType::DELETE;
                    return 1;
                }
                default : {
                    int size = get_message_size(type_byte);
                    if (size == 0) {
                        std::cerr << "Unknown message type " << type_byte << ". Stopping parse.\n";
                        out.type = MessageType::END_OF_FILE;
                        return 0;
                    }
                    file_.ignore(size - 1);
                    break;
                }
            }
        }
        out.type = MessageType::END_OF_FILE;
        return 0;
    }

    void apply_to_book(const ParsedMessage& msg, OrderBook& book) {
        switch(msg.type) {
            case MessageType::ADD : {
                const auto& m = msg.add;
                Side side = (m.buy_sell_indicator == 'B') ? Side::Buy : Side::Sell;
                Order new_order {
                    m.order_reference_id,
                    side,
                    static_cast<int64_t>(m.price),
                    m.shares,
                    0
                };
                book.add_order(new_order);
                break;
            }
            case MessageType::EXECUTE : {
                const auto& m = msg.execute;
                book.execute_order(m.order_reference_id, m.executed_shares);
                break;
            }
            case MessageType::DELETE : {
                const auto&m = msg.del;
                book.cancel_order(m.order_reference_id);
                break;
            }
            case MessageType::END_OF_FILE : {
                break;
            }
        }
    }

    void ItchParser::parse_file(const std::string& filepath, OrderBook& book) {
        ItchParser parser(filepath);
        ParsedMessage msg;
        while (parser.next(msg)) {
            apply_to_book(msg, book);
        }
    }
}