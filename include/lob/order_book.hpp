#pragma once
#include "lob/order.hpp"
#include <map>
#include <deque>
#include <unordered_map>
#include <optional>
#include <vector>

namespace lob {

    class OrderBook {
        public:
            void add_order(const Order& order);
            void cancel_order(uint64_t id);
            void modify_order(uint64_t id, uint32_t new_quantity);
            void execute_order(uint64_t id, uint32_t executed_quantity);
            std::vector<Trade> match();

            std::optional<int64_t> best_bid() const;
            std::optional<int64_t> best_ask() const;

        private:
            struct IndexEntry {
                int64_t price;
                Side side;
                uint64_t sequence;
            };

            uint64_t next_seq_{0};

            std::map<int64_t, std::deque<Order>, std::greater<int64_t>> bids_;
            std::map<int64_t, std::deque<Order>> asks_;
            std::unordered_map<uint64_t, IndexEntry> id_index_;
    };

}