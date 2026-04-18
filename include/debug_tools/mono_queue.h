#ifndef DEBUG_TOOLS_MONO_QUEUE_H
#define DEBUG_TOOLS_MONO_QUEUE_H
 
#include <atomic>
#include <cstddef>
#include <type_traits>

namespace debug_tools{
template<typename T, size_t Capacity> class alignas(64) MonoQueue{
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of two");

private:
    static constexpr size_t mask_ = Capacity - 1;

    // ===== 数据缓冲 =====
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    alignas(64) Storage buffer_[Capacity];

    // ===== head / tail 分离 cache line =====
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};

    // 填充，防止编译器合并 cache line
    char pad_[64 - sizeof(std::atomic<size_t>)];

public:
    MonoQueue() = default;

    ~MonoQueue(){
        while(!empty()) pop();
    }

    bool push(T&& value){
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t next = (tail + 1) & mask_;
        if (next == head_.load(std::memory_order_acquire)) return false;

        new (&buffer_[tail]) T(std::move(value));
        tail_.store(next, std::memory_order_release);
        return true;
    }

    bool push(const T& value){
        return push(T(value));
    }

    /**
     * @brief read only, not alter, not copy
     */
    bool top(const T*& value) const{
        const size_t head = head_.load(std::memory_order_relaxed);

        if(head == tail_.load(std::memory_order_acquire)) return false;

        value = reinterpret_cast<const T*>(&buffer_[head]);
        return true;
    }

    void pop(){
        const size_t head = head_.load(std::memory_order_relaxed);
        reinterpret_cast<T*>(&buffer_[head])->~T();
        head_.store((head + 1) & mask_, std::memory_order_release);
    }

    bool empty() const{
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }
};//class MonoQueue
}//namespace debug_tools

#endif //DEBUG_TOOLS_MONOTONIC_QUEUE_H