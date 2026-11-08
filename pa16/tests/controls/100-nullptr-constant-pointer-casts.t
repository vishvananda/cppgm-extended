// expect exit 0
// N3485 4.10/5.2.9/5.2.11: null pointer conversions do not reinterpret an object value.
constexpr const int* p=nullptr;constexpr int* q1=const_cast<int*>(p);constexpr int* q2=(int*)(const int*)nullptr;constexpr void* q3=static_cast<void*>(nullptr);using Fn=int(*)();constexpr Fn q4=static_cast<Fn>(nullptr);static_assert(q1==nullptr && q2==nullptr && q3==nullptr && q4==nullptr,"null conversions");int main(){return q1!=nullptr || q2!=nullptr || q3!=nullptr || q4!=nullptr;}
