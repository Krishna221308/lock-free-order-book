#pragma once
#include <cstdint>

namespace lob {
    
    enum class Side{
        Buy,
        Sell
    };

    struct Order {
        uint64_t id;
        Side side;
        int64_t price;
        uint32_t quantity;
        uint64_t timestamp;
    };

    struct Trade {
        uint64_t buy_id;
        uint64_t sell_id;
        int64_t price;
        uint32_t quantity;
    };
}