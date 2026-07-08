#include "lob/intrusive_bst.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>

// Wrapper to test the BST without bringing in the whole Limit structure
struct PriceNode : lob::IntrusiveBSTNode<PriceNode> {
    int64_t price;
    explicit PriceNode(int64_t p) : price(p) {}
};

// Functor required by your IntrusiveBST
struct PriceNodeKeyOf {
    int64_t operator()(const PriceNode& n) const {
        return n.price;
    }
};

int main() {
    lob::IntrusiveBST<PriceNode, int64_t, PriceNodeKeyOf> bst;
    
    // 1. Insert 20 random prices
    std::vector<int64_t> prices = { 
        45, 12, 76, 23, 89, 11, 99, 34, 56, 78,
        21, 9, 88, 33, 55, 77, 22, 10, 87, 32
    };

    std::vector<PriceNode*> nodes;
    for (int64_t p : prices) {
        PriceNode* node = new PriceNode(p);
        nodes.push_back(node);
        bst.insert(node);
    }

    assert(bst.size() == 20);

    // 2. Confirm in-order traversal yields them fully sorted
    std::vector<int64_t> in_order_result;
    PriceNode* curr = bst.min_node();
    while (curr != nullptr) {
        in_order_result.push_back(curr->price);
        curr = lob::IntrusiveBST<PriceNode, int64_t, PriceNodeKeyOf>::successor(curr);
    }

    std::vector<int64_t> expected_sorted = prices;
    std::sort(expected_sorted.begin(), expected_sorted.end());
    
    assert(in_order_result == expected_sorted);
    assert(in_order_result.size() == 20);

    // 3. Delete a node with two children (45 is our root since it was inserted first)
    PriceNode* to_delete = bst.find(45);
    assert(to_delete != nullptr);
    assert(to_delete->left != nullptr && to_delete->right != nullptr); // Confirm it has 2 children

    bst.remove(to_delete);
    delete to_delete;

    assert(bst.size() == 19);

    // 4. Confirm tree is still a valid BST with no dangling pointers
    std::vector<int64_t> post_delete_result;
    curr = bst.min_node();
    while (curr != nullptr) {
        post_delete_result.push_back(curr->price);
        curr = lob::IntrusiveBST<PriceNode, int64_t, PriceNodeKeyOf>::successor(curr);
    }

    expected_sorted.erase(std::remove(expected_sorted.begin(), expected_sorted.end(), 45), expected_sorted.end());
    assert(post_delete_result == expected_sorted);
    assert(post_delete_result.size() == 19);

    // Clean up allocated memory
    for (PriceNode* node : nodes) {
        if (node != to_delete) { 
            delete node;
        }
    }

    std::cout << "IntrusiveBST tests passed successfully!\n";
    return 0;
}
