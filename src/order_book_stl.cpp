#include "lob/order_book.hpp"
#include <algorithm>

namespace lob {

void OrderBook::add_order(const Order &order) {
    if (order.side == Side::Buy) {
        bids_[order.price].push_back(order);
    } else if (order.side == Side::Sell) {
        asks_[order.price].push_back(order);
    }
    id_index_[order.id] = IndexEntry{order.price, order.side, order.timestamp};
    // Not added a duplicate-id guard.
    // Can be implemented later tho.
}

void OrderBook::cancel_order(uint64_t id) {
    auto it = id_index_.find(id);
    if (it == id_index_.end()) {
        return;
    }

    int64_t price = it->second.price;
    Side side = it->second.side;

    if (side == Side::Buy) {
        auto level_it = bids_.find(price);
        if (level_it == bids_.end()) {
            id_index_.erase(it);
            return;
        }

        auto &level = level_it->second;

        auto order_it =
            std::find_if(level.begin(), level.end(), [id](const Order &o) { return o.id == id; });
        if (order_it == level.end()) {
            id_index_.erase(it);
            return; // Guard for what if the order does not exist.
        }
        level.erase(order_it);

        if (level.empty()) {
            bids_.erase(level_it);
        }
    } else {
        auto level_it = asks_.find(price);
        if (level_it == asks_.end()) {
            id_index_.erase(id);
            return;
        }

        auto &level = level_it->second;

        auto order_it =
            std::find_if(level.begin(), level.end(), [id](const Order &o) { return o.id == id; });
        if (order_it == level.end()) {
            id_index_.erase(it);
            return; // Guard for what if the order does not exist.
        }
        level.erase(order_it);

        if (level.empty()) {
            asks_.erase(level_it);
        }
    }

    id_index_.erase(it);
}

void OrderBook::modify_order(uint64_t id, uint32_t new_quantity) {
    // Modifying modify_order to change the priority and push it back instead of
    // keeping it at the same place to match with the intrusive data structure
    // implementation.
    if (new_quantity == 0) {
        cancel_order(id);
        return;
    }

    auto it = id_index_.find(id);
    if (it == id_index_.end())
        return;

    int64_t price = it->second.price;
    Side side = it->second.side;
    uint64_t timestamp = it->second.timestamp;

    if (side == Side::Buy) {
        auto level_it = bids_.find(price);
        if (level_it == bids_.end()) {
            id_index_.erase(it);
            return;
        }

        auto &level = level_it->second;

        auto order_it =
            std::find_if(level.begin(), level.end(), [id](const Order &o) {
                return o.id == id;
            });
        if (order_it == level.end()) {
            id_index_.erase(it);
            return;
        }
        level.erase(order_it);
        level.push_back({id, side, price, new_quantity, timestamp});
    } else {
        auto level_it = asks_.find(price);
        if (level_it == asks_.end()) {
            id_index_.erase(it);
            return;
        }

        auto &level = level_it->second;

        auto order_it =
            std::find_if(level.begin(), level.end(), [id](const Order &o) {
                return o.id == id;
            });
        if (order_it == level.end()) {
            id_index_.erase(it);
            return;
        }
        level.erase(order_it);
        level.push_back({id, side, price, new_quantity, timestamp});
    }
}

void OrderBook::match() {
    while (!bids_.empty() && !asks_.empty() &&
           (bids_.begin()->first >= asks_.begin()->first)) {
        const Order &bid_front = bids_.begin()->second.front();
        const Order &ask_front = asks_.begin()->second.front();

        uint32_t trade_quantity =
            std::min(bid_front.quantity, ask_front.quantity);

        modify_order(bid_front.id, bid_front.quantity - trade_quantity);
        modify_order(ask_front.id, ask_front.quantity - trade_quantity);
    }
}

std::optional<int64_t> OrderBook::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.begin()->first;
}

std::optional<int64_t> OrderBook::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}
} // namespace lob