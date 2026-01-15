#ifndef DEBUG_TOOLS_TIME_DISPATCHER_H
#define DEBUG_TOOLS_TIME_DISPATCHER_H

#include "debug_tools/mono_queue.h"
#include <tuple>
#include <limits> 

namespace debug_tools{
template<typename T, typename Tuple>
struct TupleIndex;

template<typename T, typename... Ts>
struct TupleIndex<T, std::tuple<T, Ts...>>{
    static constexpr size_t value = 0;
};

template<typename T, typename U, typename... Ts>
struct TupleIndex<T, std::tuple<U, Ts...>>{
    static constexpr size_t value = 1 + TupleIndex<T, std::tuple<Ts...>>::value;
};

template<size_t Capacity, typename... Msgs>
class TimeDispatcher{
private:
    template<typename T> using Queue = MonoQueue<T, Capacity>;
    std::tuple<Queue<Msgs>...> queues_;

    static constexpr size_t invalid_index_ = static_cast<size_t>(-1);

    /**
     * @brief scanning all element.top(), find minner timestamp
     */
    template<size_t I>
    void scanTop(uint64_t& min_ts, size_t& min_idx){
        if constexpr (I < sizeof...(Msgs)){
            const auto& q = std::get<I>(queues_);
            using MsgType = typename std::tuple_element<I, std::tuple<Msgs...>>::type;
            const MsgType* ptr = nullptr;

            if(q.top(ptr)){
                uint64_t ts = ptr->timestamp;

                if(ts < min_ts){
                    min_ts = ts;
                    min_idx = I;
                }
            }
            scanTop<I + 1>(min_ts, min_idx);
        }
    }

public:
    TimeDispatcher() = default;

    template<typename Msg>
    bool push(Msg msg){
        using Pure = std::remove_cv_t<std::remove_reference_t<Msg>>;
        constexpr size_t idx = TupleIndex<Pure, std::tuple<Msgs...>>::value;
        return std::get<idx>(queues_).push(std::move(msg));
    }

    /**
     * @brief give priority to returning elements with earlier timerstamps
     */
    template<typename Msg> bool tryPop(Msg& out){
        constexpr size_t my_idx = TupleIndex<Msg, std::tuple<Msgs...>>::value;

        uint64_t min_ts = std::numeric_limits<uint64_t>::max();
        size_t min_idx = invalid_index_;
        scanTop<0>(min_ts, min_idx);

        if (min_idx != my_idx) return false;

        auto& q = std::get<my_idx>(queues_);
        const Msg* ptr;
        if (!q.top(ptr)) return false;

        out = std::move(*ptr);
        q.pop();
        return true;
    }

};//class TimeDispatcher
}//namespace debug_tools

#endif //DEBUG_TOOLS_TIME_DISPATCHER_H