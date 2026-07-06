#pragma once
#include <cstddef>

namespace lob {
    template <typename T>
    struct IntrusiveListNode {
        T* prev = nullptr;
        T* next = nullptr;
    };

    template <typename T>
    class IntrusiveList {
        public:
            IntrusiveList() = default;
            
            IntrusiveList(const IntrusiveList&) = delete;
            IntrusiveList& operator=(const IntrusiveList&) = delete;

            bool empty() const { return head_ == nullptr; }
            T* front() const { return head_; }
            T* back() const { return tail_; }

            void push_back(T* node) {
                node->prev = tail_;
                node->next = nullptr;
                if (tail_ != nullptr) {
                    tail_->next = node;
                }
                else {
                    head_ = node;
                }
                tail_ = node;
                ++size_;
            }

            void insert_after(T* pos, T*node) {
                if (pos == nullptr) {
                    node->prev = nullptr;
                    node->next = head_;
                    if (head_ != nullptr) {
                        head_->prev = node;
                    } else {
                        tail_ = node;
                    }
                    head_ = node;
                }
                else {
                    node->prev = pos;
                    node->next = pos->next;
                    pos->next = node;
                    if (node->next == nullptr) {
                        tail_ = node;
                    } 
                    else {
                        node->next->prev = node;
                    }
                }
                ++size_;
            }

            void remove(T* node) {
                if (node->prev == nullptr) {
                    head_ = node->next;
                }
                else {
                    node->prev->next = node->next;
                }

                if (node->next == nullptr) {
                    tail_ = node->prev;
                }
                else {
                    node->next->prev = node->prev;
                }
                node->prev = nullptr;
                node->next = nullptr;
                --size_;
            }

            std::size_t size() const{ return size_; }

            class iterator {
                public:
                    explicit iterator(T* node) : node_(node) {}
                    T* operator*() const { return node_; }
                    iterator& operator++() {
                        node_ = node_->next;
                        return *this;
                    }
                    bool operator!=(const iterator& other) const{ return node_ != other.node_; }
                
                private:
                    T* node_;
            };

            iterator begin() const { return iterator(head_); }
            iterator end() const { return iterator(nullptr); }

            class reverse_iterator {
                public:
                    explicit reverse_iterator(T* node) : node_(node) {}
                    T* operator*() const { return node_; }
                    reverse_iterator& operator++() {
                        node_ = node->prev;
                        return *this;
                    }
                    bool operator!=(const reverse_iterator& other) const { return node_ != other.node; }

                private:
                    T* node_;
            };

            reverse_iterator rbegin() const { return reverse_iterator(tail_); }
            reverse_iterator rend() const { return reverse_iterator(nullptr); }

        private:
            T* head_ = nullptr;
            T* tail_ = nullptr;
            std::size_t size_ = 0;
    };
}