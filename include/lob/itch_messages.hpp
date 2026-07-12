#pragma once
#include <cstdint>

namespace lob {
    enum class MessageType : uint8_t {
        ADD = 'A',
        EXECUTE = 'E',
        DELETE = 'D'
    };

    struct AddOrderMessage {
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_id;
        char buy_sell_indicator;
        uint32_t shares;
        char stock[8];
        uint32_t price;
    };

    struct ExecuteOrderMessage {
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_id;
        uint32_t executed_shares;
        uint64_t match_number;
    };

    struct DeleteOrderMessage {
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_id;
    };

    struct ParsedMessage {
        MessageType type = MessageType::END_OF_FILE;
        AddOrderMessage add{};
        ExecuteOrderMessage execute{};
        DeleteOrderMessage del{};
    };
}