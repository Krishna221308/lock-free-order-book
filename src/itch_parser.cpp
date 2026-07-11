#include "lob/itch_parser.hpp"
#include <fstream>
#include <vector>
#include <cstring>

namespace lob {
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

    void ItchParser::parse_file(const std::string& filepath, OrderBook& book) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open ITCH file: " << filepath << '\n';
            return;
        }

        char type_byte;
        while (file.read(&type_byte, 1)) {
            switch (type_byte) {
                case 'A': {
                    std::vector<uint8_t> buffer(35);
                    file.read(reinterpret_cast<char*> (buffer.data()), 35);
                    AddOrderMessage msg;

                    // Filling the details of the struct.
                    msg.stock_locate = to_uint16_be(buffer.data());
                    msg.tracking_number = to_uint16_be(buffer.data() + 2);
                    msg.timestamp = to_uint48_be(buffer.data() + 4);
                    msg.order_reference_id = to_uint64_be(buffer.data() + 10);
                    msg.buy_sell_indicator = static_cast<char> (buffer[18]);
                    msg.shares = to_uint32_be(buffer.data() + 19);;
                    std::memcpy(msg.stock, buffer.data() + 23, 8);
                    msg.price = to_uint32_be(buffer.data() + 31);

                    Side side = (msg.buy_sell_indicator == 'B') ? Side::Buy : Side::Sell;
                    Order new_order{
                        msg.order_reference_id,
                        side,
                        static_cast<int64_t> (msg.price),
                        msg.shares,
                        0
                    };

                    book.add_order(new_order);
                    break;
                }
                case 'E' :{
                    std::vector<uint8_t> buffer(30);
                    file.read(reinterpret_cast<char*> (buffer.data()), 30);
                    ExecuteOrderMessage msg;

                    // Filling in the details of the struct
                    msg.stock_locate = to_uint16_be(buffer.data());
                    msg.tracking_number = to_uint16_be(buffer.data() + 2);
                    msg.timestamp = to_uint48_be(buffer.data() + 4);
                    msg.order_reference_id = to_uint64_be(buffer.data() + 10);
                    msg.executed_shares = to_uint32_be(buffer.data() + 18);
                    msg.match_number = to_uint64_be(buffer.data() + 22);
                    
                    book.execute_order(msg.order_reference_id, msg.executed_shares);

                    break;
                }
                case 'D' :{
                    std::vector<uint8_t> buffer(30);
                    file.read(reinterpret_cast<char*> (buffer.data()), 18);
                    DeleteOrderMessage msg;

                    // Filling in the details of the struct
                    msg.stock_locate = to_uint16_be(buffer.data());
                    msg.tracking_number = to_uint16_be(buffer.data() + 2);
                    msg.timestamp = to_uint48_be(buffer.data() + 4);
                    msg.order_reference_id = to_uint64_be(buffer.data() + 10);

                    book.cancel_order(msg.order_reference_id);
                    break;
                }
                default : {
                    int size = get_message_size(type_byte);
                    if (size == 0) {
                        std::cerr << "Unknown message type: " << type_byte << "Stopping parse.\n";
                        return;
                    } file.ignore(size - 1);
                    break;
                }
            }
        }
    }
}