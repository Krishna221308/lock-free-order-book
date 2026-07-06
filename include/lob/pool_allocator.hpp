#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>

namespace lob {
    template <typename T>
    class ObjectPool {
        private:
            union Slot {
                Slot* next_free;
                alignas(T) unsigned char storage[sizeof(T)];
            };

            Slot* slot_ad(std::size_t i) { return storage_ + i; }
            
            Slot* storage_ = nullptr;
            Slot* free_head_ = nullptr;
            std::size_t capacity_ = 0;
            std::size_t live_count_ = 0;

        public: 
            explicit ObjectPool(std::size_t capacity) : capacity_(capacity) {
                storage_ = static_cast<Slot*> (::operator new(capacity_ * sizeof(Slot), std::align_val_t(alignof(Slot))));
                for (std::size_t i = 0; i + 1 < capacity_; ++i) {
                    slot_at(i)->next_free = slot_at(i + 1);
                }
                if (capacity_ > 0) {
                    slot_at(capacity_ - 1)->next_free = nullptr;
                    free_head_ = slot_at(0);
                }
            }

            ~ObjectPool() {
                ::operator delete(storage_, std::align_val_t(alignof(Slot)));
            }

            ObjectPool(const ObjectPool&) = delete;
            ObjectPool& operator=(const ObjectPool&) = delete;

            template <typename... Args>
            T* acquire(Args&&... args) {
                if (free_head_ == nullptr) {
                    return nullptr;
                }
                Slot* slot = free_head_;
                free_head_ = slot-> next_free;
                T* obj = ::new(static_cast<void*>(slot)) T(static_cast<Args&&>(args)...);
                ++live_count_;
                return obj;
            }

            void release(T* obj) {
                assert(obj != nullptr) 
                obj->~T();
                Slot* slot = reinterpret_cast<Slot*>(obj);
                slot->next_free = free_head_;
                free_head_ = slor;
                --live_count_;                
            }

            std::size_t capacity() const { return capacity_; }
            std::size_t live_count() const { return live_count_; }
            std::size_t free_count() const { return capacity_ - live_count_; }
            
    };



}