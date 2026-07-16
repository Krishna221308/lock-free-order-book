#pragma once
#include <cstddef>

/*
 Since we will be applying some operations we define some naming to classify the cases
 i -> where to insert
 o -> empty
 x -> occupied
 ? -> can be 'o' or 'x'
 r -> where to remove
 it will be of format ? ? ? showing the three cases where middle ? is generally the i
*/

namespace lob {
    template <typename T>
    struct IntrusiveListNode {
        T* prev = nullptr;
        T* next = nullptr;
    };

    template <typename T>
    class IntrusiveList {
        private:
            T* head_ = nullptr;
            T* tail_ = nullptr;
            std::size_t size_ = 0;

        public:
            IntrusiveList() = default;
            
            // Copy Constructor and Copy assignment operator to forbid copying and overwriting
            IntrusiveList(const IntrusiveList&) = delete;
            IntrusiveList& operator=(const IntrusiveList&) = delete;

            bool empty() const { return (head_ == nullptr && size_ == 0 && tail_ == nullptr); }     // Safety check added all 3 should be satisfied.
            T* front() const { return head_; }
            T* back() const { return tail_; }

            void push_back(T* node) {       // ? i o
                node->prev = tail_;         
                node->next = nullptr;       
                if (tail_ != nullptr) {     // o i o
                    tail_->next = node;
                }
                else {                      // x i o
                    head_ = node;
                }
                tail_ = node;
                ++size_;
            }

            void insert_after(T* pos, T*node) {
                if (pos == nullptr) {               // o i ?
                    node->prev = nullptr;
                    node->next = head_;
                    if (head_ != nullptr) {         // o i x
                        head_->prev = node;
                    } else {                        // o i o
                        tail_ = node;   
                    }
                    head_ = node;
                }
                else {                              // x i ?
                    node->prev = pos;
                    node->next = pos->next;
                    pos->next = node;
                    if (node->next == nullptr) {    // x i o
                        tail_ = node;
                    } 
                    else {                          // x i x
                        node->next->prev = node;
                    }
                }
                ++size_;
            }

            void remove(T* node) {                  // ? r ?
                if (node->prev == nullptr) {        // o r ?
                    head_ = node->next;
                }
                else {                              // x r ?
                    node->prev->next = node->next;
                }

                if (node->next == nullptr) {        // ? r o
                    tail_ = node->prev;
                }
                else {                              // ? r x
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
                        node_ = node_->prev;
                        return *this;
                    }
                    bool operator!=(const reverse_iterator& other) const { return node_ != other.node_; }

                private:
                    T* node_;
            };

            reverse_iterator rbegin() const { return reverse_iterator(tail_); }
            reverse_iterator rend() const { return reverse_iterator(nullptr); }

    };
}