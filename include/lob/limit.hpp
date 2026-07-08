#pragma once
#include "lob/intrusive_list.hpp"
#include "lob/intrusive_bst.hpp"
#include "lob/intrusive_order.hpp"
#include <cstdint>

namespace lob {
    struct Limit;

    struct LimitKeyOf {
        int64_t operator()(const Limit& l) const;
    };

    struct Limit : IntrusiveBSTNode<Limit> {
        int64_t price = 0;
        IntrusiveList<IntrusiveOrder> orders;
    };

    inline int64_t LimitKeyOf::operator()(const Limit& l) const {
        return l.price;
    }
}