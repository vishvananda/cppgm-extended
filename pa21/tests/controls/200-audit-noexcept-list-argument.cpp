// AUDIT-ID: NOEXCEPT-LIST
// AUDIT-EXPECT: compile
namespace std {
template<class E> class initializer_list {
 const E* first; unsigned long count;
 initializer_list(const E* p,unsigned long n):first(p),count(n){}
 public:
 initializer_list():first(0),count(0){}
 unsigned long size()const{return count;}
 const E* begin()const{return first;}
 const E* end()const{return first+count;}
};
}
struct Item { Item(int) noexcept {} };
struct Box { Box(std::initializer_list<Item>) noexcept {} };
int consume(const Box&) noexcept;
static_assert(noexcept(consume({1,2})), "nonthrowing list argument");
int main() {}
