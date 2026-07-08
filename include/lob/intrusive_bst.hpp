#pragma once
#include <cstddef>

namespace lob {
    template<typename T>
    struct IntrusiveBSTNode {
        T* left = nullptr;
        T* right = nullptr;
        T* parent = nullptr;
    };

    template<typename T, typename Key, typename KeyOf>
    class IntrusiveBST {
        private:
            T* root_ = nullptr;
            std::size_t size_ = 0;

            void replace_in_parent(T* node, T* replacement) {
                T* parent = node->parent;
                if (parent == nullptr) {
                    root_ = replacement;
                }
                else if (parent->left == node) {
                    parent->left = replacement;
                }
                else {
                    parent->right = replacement;
                }
                if (replacement != nullptr) {
                    replacement->parent = parent;
                }
            }

            void transplant_value_holder(T* node, T* succ) {
                T* succ_parent = succ->parent;
                T* succ_right = succ->right;
                if (succ_parent != node) {
                    replace_in_parent(succ, succ_right);
                    succ->right = node->right;
                    if (succ->right != nullptr) succ->right->parent = succ;
                }
                replace_in_parent(node, succ);
                succ->left = node->left;
                if (succ->left != nullptr) {
                    succ->left->parent = succ;
                }
            }

        public:
            IntrusiveBST() = default;
            IntrusiveBST(const IntrusiveBST&) = delete;
            IntrusiveBST& operator=(const IntrusiveBST&) = delete;

            bool empty() const { return root_ == nullptr; }
            std::size_t size() const { return size_; }

            void insert(T* node) {
                node->left = nullptr;
                node->right = nullptr;
                node->parent = nullptr;

                if (root_ == nullptr) {
                    root_ = node;
                    ++size_;
                    return;
                }

                T* cur = root_;
                Key key = KeyOf{}(*node);
                while (true) {
                    if (key < KeyOf{}(*cur)) {
                        if (cur->left == nullptr) {
                            cur->left = node;
                            node->parent = cur;
                            break;
                        }
                        cur = cur->left;
                    } else {
                        if (cur->right == nullptr) {
                            cur->right = node;
                            node->parent = cur;
                            break;
                        }
                        cur = cur->right;
                    }
                } 
                ++size_;
            }

            T* find(const Key& key) const {
                T* cur = root_;
                while (cur != nullptr) {
                    Key cur_key = KeyOf{}(*cur);
                    if (key < cur_key) {
                        cur = cur->left;
                    }
                    else if (key > cur_key){
                        cur = cur->right;
                    } else {
                        return cur;
                    }
                }
                return nullptr;
            }

            T* min_node() const {
                if (root_ == nullptr) return nullptr;
                T* cur = root_;
                while (cur->left != nullptr) cur = cur->left;
                return cur;
            }

            T* max_node() const {
                if (root_ == nullptr) return nullptr;
                T* cur = root_;
                while (cur->right != nullptr) cur = cur->right;
                return cur;
            }

            static T* successor(T* node) {
                if (node->right != nullptr) {
                    T* cur = node->right;
                    while (cur->left != nullptr) cur = cur->left;
                    return cur;
                }
                T* cur = node;
                T* parent = cur->parent;
                while (parent != nullptr && cur == parent->right) {
                    cur = parent;
                    parent = cur->parent;
                }
                return parent;
            }

            static T* predecessor(T* node) {
                if (node->left != nullptr) {
                    T* cur = node->left;
                    while (cur->right != nullptr) cur = cur->right;
                    return cur;
                }

                T* cur = node;
                T* parent = cur->parent;
                while (parent != nullptr && cur == parent->left) {
                    cur = parent;
                    parent = cur->parent;
                }
                return parent;
            }

            void remove(T* node) {
                if (node->left != nullptr && node->right != nullptr) {
                    T* succ = successor(node);
                    transplant_value_holder(node, succ);
                }
                else {
                    T* child = (node->left != nullptr) ? node->left : node->right;
                    replace_in_parent(node, child);
                }
                node->left = nullptr;
                node->right = nullptr;
                node->parent = nullptr;
                --size_;
            }
    };
}