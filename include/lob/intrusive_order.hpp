#pragma once
#include "lob/order.hpp"
#include "lob/intrusive_list.hpp"
#include "lob/intrusive_bst.hpp"
#include <cstdint>

namespace lob {
    struct IntrusiveOrder : IntrusiveListNode<IntrusiveOrder> {
        uint64_t id;
        Side side;
        int64_t price;
        uint32_t quantity;
        uint64_t timestamp;
    };
}