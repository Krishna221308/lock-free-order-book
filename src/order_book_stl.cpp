#include "lob/order_book.hpp"
#include <algorithm>

namespace lob {

void OrderBook::add_order(const Order &order) {
    next_seq_++;
    if (id_index_.count(order.id)) return;      // Duplicate id guard;
    Order custom = order;
    custom.timestamp = next_seq_;
    if (order.side == Side::Buy) {
        bids_[order.price].push_back(custom);
    } else if (order.side == Side::Sell) {
        asks_[order.price].push_back(custom);
    }
    id_index_[order.id] = IndexEntry{order.price, order.side, next_seq_};
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
    
    if (side == Side::Buy) {
        auto level_it = bids_.find(price);
        if (level_it == bids_.end()) {
            id_index_.erase(it);
            return;
        }

        auto& level = level_it->second;

        auto order_it = std::find_if(level.begin(), level.end(), [id](const Order &o) {
            return o.id == id;
        });
        if (order_it == level.end()) {
            id_index_.erase(it);
            return;
        }

        if (order_it->quantity >= new_quantity) {
            order_it->quantity = new_quantity;
        } else {
            next_seq_++;
            it->second.sequence = next_seq_;
            level.erase(order_it);
            level.push_back({id, side, price, new_quantity, next_seq_});
        }
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
        if (order_it->quantity >= new_quantity) {
            order_it->quantity = new_quantity;
        } else {
            next_seq_++;
            it->second.sequence = next_seq_;
            level.erase(order_it);
            level.push_back({id, side, price, new_quantity, next_seq_});
        }
    }
}

std::vector<Trade> OrderBook::match() {

    std::vector<Trade> trades;
    while (!bids_.empty() && !asks_.empty() && (bids_.begin()->first >= asks_.begin()->first)) {
        const Order &bid_front = bids_.begin()->second.front();
        const Order &ask_front = asks_.begin()->second.front();

        uint32_t trade_quantity = std::min(bid_front.quantity, ask_front.quantity);

        int64_t price = (bid_front.timestamp < ask_front.timestamp) ? bid_front.price : ask_front.price;

        trades.push_back({bid_front.id, ask_front.id, price, trade_quantity});
        
        modify_order(bid_front.id, bid_front.quantity - trade_quantity);
        modify_order(ask_front.id, ask_front.quantity - trade_quantity);

    }

    return trades;
}

void OrderBook::execute_order(uint64_t id, uint32_t executed_quantity) {
    auto it = id_index_.find(id);
    if (it == id_index_.end()) return;

    int64_t price = it->second.price;
    Side side = it->second.side;

    if (side == Side::Buy) {
        auto level_it = bids_.find(price);
        if (level_it == bids_.end()) return;

        auto &level = level_it->second;
        auto order_it = std::find_if(level.begin(), level.end(), [id](const Order &o) {
            return o.id == id;
        });
        if (order_it == level.end()) return;

        if (executed_quantity >= order_it->quantity) {
            level.erase(order_it);
            if (level.empty()) {
                bids_.erase(level_it);
            }
            id_index_.erase(it);
        } else {
            order_it->quantity -= executed_quantity;
        }
    } else {
        auto level_it = asks_.find(price);
        if (level_it == asks_.end()) return;

        auto &level = level_it->second;
        auto order_it = std::find_if(level.begin(), level.end(), [id](const Order &o) {
            return o.id == id;
        });
        if (order_it == level.end()) return;

        if (executed_quantity >= order_it->quantity) {
            level.erase(order_it);
            if (level.empty()) {
                asks_.erase(level_it);
            }
            id_index_.erase(it);
        } else {
            order_it->quantity -= executed_quantity;
        }
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