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

}