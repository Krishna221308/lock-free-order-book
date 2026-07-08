#include "lob/order_book_intrusive.hpp"
#include <algorithm>

namespace lob {
    IntrusiveOrderBook::~IntrusiveOrderBook() {
        for (auto& pair : id_index_) delete pair.second;
        for (auto& pair : limit_index_) delete pair.second;
    }

    void IntrusiveOrderBook::add_order(const Order& order) {
        Limit* limit = nullptr;
        auto limit_it = limit_index_.find(order.price);

        if (limit_it != limit_index_.end()) {
            limit = limit_it->second;
        }
        else {
            limit = new Limit();
            limit->price = order.price;
            limit_index_[order.price] = limit;

            if (order.side == Side::Buy) {
                bids_.insert(limit);
            }
            else {
                asks_.insert(limit);
            }
        }
        IntrusiveOrder* intrusive_order = new IntrusiveOrder();
        intrusive_order->id = order.id;
        intrusive_order->side = order.side;
        intrusive_order->price = order.price;
        intrusive_order->quantity = order.quantity;
        intrusive_order->timestamp = order.timestamp;

        limit->orders.push_back(intrusive_order);
        id_index_[order.id] = intrusive_order;
    }

    void IntrusiveOrderBook::cancel_order(uint64_t id) {
        auto it = id_index_.find(id);
        if (it == id_index_.end()) return;

        IntrusiveOrder* intrusive_order = it->second;
        Limit* limit = limit_index_[intrusive_order->price];
        Side order_side = intrusive_order->side;

        limit->orders.remove(intrusive_order);
        id_index_.erase(id);
        delete intrusive_order;

        if (limit->orders.empty()) {
            if (order_side == Side::Buy) bids_.remove(limit);
            else asks_.remove(limit);
            limit_index_.erase(limit->price);
            delete limit;
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
        intrusive_order->quantity = new_quantity;

        // It loses time priority due to modification.
        Limit* limit = limit_index_[intrusive_order->price];
        limit->orders.remove(intrusive_order);
        limit->orders.push_back(intrusive_order);
    }

    void IntrusiveOrderBook::match() {
        while (!bids_.empty() && !asks_.empty()) {
            Limit* best_bid_limit = bids_.max_node();
            Limit* best_ask_limit = asks_.min_node();
            
            if (best_bid_limit < best_ask_limit) break;

            IntrusiveOrder* bid_order = best_bid_limit->orders.front();
            IntrusiveOrder* ask_order = best_ask_limit->orders.front();

            uint32_t traded = std::min(bid_order->quantity, ask_order->quantity);
            
            bid_order->quantity -= traded;
            ask_order->quantity -= traded;

            if (bid_order->quantity == 0) cancel_order(bid_order->id);
            else cancel_order(ask_order->id);
        }
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