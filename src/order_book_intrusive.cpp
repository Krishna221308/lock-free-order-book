#include "lob/order_book_intrusive.hpp"
#include <algorithm>

namespace lob {
    IntrusiveOrderBook::~IntrusiveOrderBook() {
        for (auto& pair : id_index_) order_pool_.release(pair.second);
        for (auto& pair : bid_limit_index_) limit_pool_.release(pair.second);
        for (auto& pair : ask_limit_index_) limit_pool_.release(pair.second);
    }

    void IntrusiveOrderBook::add_order(const Order& order) {
        if (id_index_.find(order.id) != id_index_.end()) return;

        Limit* limit = nullptr;
        auto& limit_index = (order.side == Side::Buy) ? bid_limit_index_ : ask_limit_index_;
        auto limit_it = limit_index.find(order.price);

        next_seq_++;

        if (limit_it != limit_index.end()) {
            limit = limit_it->second;
        }
        else {
            limit = limit_pool_.acquire();
            limit->price = order.price;
            limit_index[order.price] = limit;

            if (order.side == Side::Buy) {
                bids_.insert(limit);
            }
            else {
                asks_.insert(limit);
            }
        }
        IntrusiveOrder* intrusive_order = order_pool_.acquire();
        intrusive_order->id = order.id;
        intrusive_order->side = order.side;
        intrusive_order->price = order.price;
        intrusive_order->quantity = order.quantity;
        intrusive_order->timestamp = next_seq_;

        limit->orders.push_back(intrusive_order);
        id_index_[order.id] = intrusive_order;
    }

    void IntrusiveOrderBook::cancel_order(uint64_t id) {
        auto it = id_index_.find(id);
        if (it == id_index_.end()) return;

        IntrusiveOrder* intrusive_order = it->second;
        auto& limit_index = (intrusive_order->side == Side::Buy) ? bid_limit_index_ : ask_limit_index_;
        Limit* limit = limit_index[intrusive_order->price];
        Side order_side = intrusive_order->side;

        limit->orders.remove(intrusive_order);
        id_index_.erase(id);
        order_pool_.release(intrusive_order);

        if (limit->orders.empty()) {
            if (order_side == Side::Buy) bids_.remove(limit);
            else asks_.remove(limit);
            limit_index.erase(limit->price);
            limit_pool_.release(limit);
        }
    }

    void IntrusiveOrderBook::modify_order(uint64_t id, uint32_t new_quantity) {
        if (new_quantity == 0) {
            cancel_order(id);
            return;
        }

        auto it = id_index_.find(id);
        if (it == id_index_.end()) return;

        IntrusiveOrder* intrusive_order = it->second;
        if (intrusive_order->quantity >= new_quantity) {
            intrusive_order->quantity = new_quantity;
            return;
        }

        next_seq_++;
        intrusive_order->timestamp = next_seq_;
        intrusive_order->quantity = new_quantity;

        // It loses time priority due to modification.
        Side order_side = intrusive_order->side;
        auto& limit_index = (order_side == Side::Buy) ? bid_limit_index_ : ask_limit_index_;
        Limit* limit = limit_index[intrusive_order->price];
        limit->orders.remove(intrusive_order);
        limit->orders.push_back(intrusive_order);
    }

    std::vector<Trade> IntrusiveOrderBook::match() {
        std::vector<Trade> trades_intrusive;

        while (!bids_.empty() && !asks_.empty()) {
            Limit* best_bid_limit = bids_.max_node();
            Limit* best_ask_limit = asks_.min_node();
            
            if (best_bid_limit->price < best_ask_limit->price) break;

            IntrusiveOrder* bid_order = best_bid_limit->orders.front();
            IntrusiveOrder* ask_order = best_ask_limit->orders.front();

            uint32_t traded = std::min(bid_order->quantity, ask_order->quantity);
            
            int64_t price = (bid_order->timestamp < ask_order->timestamp) ? bid_order->price : ask_order->price;
            trades_intrusive.push_back({bid_order->id, ask_order->id, price, traded});

            bid_order->quantity -= traded;
            ask_order->quantity -= traded;

            if (bid_order->quantity == 0) cancel_order(bid_order->id);
            if (ask_order->quantity == 0) cancel_order(ask_order->id);          // What if there is an exact match
        }

        return trades_intrusive;
    }

    void IntrusiveOrderBook::execute_order(uint64_t id, uint32_t executed_quantity) {
        auto it = id_index_.find(id);
        if (it == id_index_.end()) return;

        IntrusiveOrder* intrusive_order = it->second;
        if (intrusive_order->quantity <= executed_quantity) {
            cancel_order(id);
            return;
        }
        
        intrusive_order->quantity -= executed_quantity;
    }

    std::optional<int64_t> IntrusiveOrderBook::best_bid() const {
        Limit* max_bid = bids_.max_node();
        if (max_bid != nullptr) return max_bid->price;
        return std::nullopt;
    }

    std::optional<int64_t> IntrusiveOrderBook::best_ask() const {
        Limit* min_ask = asks_.min_node();
        if (min_ask != nullptr) return min_ask->price;
        return std::nullopt;
    }
}