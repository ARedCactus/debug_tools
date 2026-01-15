#include "debug_tools/mono_queue.h"
#include <thread>
#include <iostream>
#include <opencv2/opencv.hpp>

struct SpecialValue{
    int a, b;
};//struct SpecialValue

struct Element{
    SpecialValue v_1;
    std::shared_ptr<SpecialValue> v_2;
    size_t s;
};//struct Element

debug_tools::MonoQueue<Element, 1024> que_;

int main(){
    std::thread producer([](){
        for(int i{0}; i < 100; ++i){
            SpecialValue s_v{1, 2};
            auto s_v_ptr = std::make_shared<SpecialValue>();
            s_v_ptr->a = 3;
            s_v_ptr->b = 4;
            
            while(!que_.push(Element{s_v, s_v_ptr, static_cast<size_t>(i+100)})){}
        }
    });

    std::thread consumer([](){
        for(int i{0}; i < 100; i++){
            const Element* e;
            if(que_.top(e)){
                std::cout << e->v_1.a << " " << e->v_1.b << " " 
                        << e->v_2->a << " " << e->v_2->b << " " << e->s << "\n";
                que_.pop();
            }
        }
        std::cout << std::endl;
    });

    producer.join();
    consumer.join();

    return 0;
}



