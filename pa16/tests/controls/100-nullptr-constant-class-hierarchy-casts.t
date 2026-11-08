// expect exit 0
// N3485 4.10/5.2.9/5.2.11: null pointer conversions do not reinterpret an object value.
struct B{int member;};struct D:B{int other;};constexpr B* b=static_cast<B*>(nullptr);constexpr D* d=nullptr;constexpr B* up=static_cast<B*>(d);constexpr D* down=static_cast<D*>(b);static_assert(b==nullptr && up==nullptr && down==nullptr,"null cast has no subobject offset");int main(){return b!=nullptr || up!=nullptr || down!=nullptr;}
