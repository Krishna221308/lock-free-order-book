#include "lob/itch_parser.hpp"
#include <fstream>
#include <vector>

namespace lob {
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
                    std::vector<uint8_t*> buffer(35);
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
            }
        }
    }
}