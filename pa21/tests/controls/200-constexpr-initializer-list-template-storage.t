// expect exit 0
// N3485 8.5.4p5/5.19: static/local/template list objects retain the constant backing array.
typedef decltype(sizeof(0)) Size; namespace std { template<class T> struct initializer_list { const T* data; Size count; constexpr Size size() const { return count; } constexpr const T* begin() const { return data; } }; }
constexpr std::initializer_list<int> global={2,3}; static_assert(global.size()==2 && global.begin()[1]==3,"global backing");
template<int> void inspect() { static constexpr std::initializer_list<int> local{4,5,6}; static_assert(local.size()==3 && local.begin()[2]==6,"local backing"); }
template<int> struct Holder { static constexpr std::initializer_list<int> list={7,8}; static_assert(list.size()==2 && list.begin()[0]==7,"member backing"); };
int main() { inspect<0>(); Holder<1> value; return global.begin()[0]!=2; }
