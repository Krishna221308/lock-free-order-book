#pragma once
#include "lob/order.hpp"
#include "lob/intrusive_list.hpp"
#include "lob/intrusive_bst.hpp"
#include "lob/intrusive_order.hpp"
#include "lob/limit.hpp"
#include "lob/pool_allocator.hpp"
#include <unordered_map>
#include <optional>
#include <vector>

namespace lob {
    class IntrusiveOrderBook {
        private:
            IntrusiveBST<Limit, int64_t, LimitKeyOf> bids_;
            IntrusiveBST<Limit, int64_t, LimitKeyOf> asks_;
            std::unordered_map<uint64_t, IntrusiveOrder*> id_index_;
            std::unordered_map<int64_t, Limit*> bid_limit_index_;
            std::unordered_map<int64_t, Limit*> ask_limit_index_;

            ObjectPool<IntrusiveOrder> order_pool_{1000000};  // Pool for 1M orders.
            ObjectPool<Limit> limit_pool_{10000};             // Pool for 10K prices

            uint64_t next_seq_{0};

        public:
            IntrusiveOrderBook() = default;
            ~IntrusiveOrderBook();

            IntrusiveOrderBook(const IntrusiveOrderBook&) = delete;
            IntrusiveOrderBook& operator=(const IntrusiveOrderBook&) = delete;

            void add_order(const Order& order);
            void cancel_order(uint64_t id);
            void modify_order(uint64_t id, uint32_t new_quantity);
            std::vector<Trade> match();
            void execute_order(uint64_t id, uint32_t executed_quantity);

            std::optional<int64_t> best_bid() const;
            std::optional<int64_t> best_ask() const;
    };
}