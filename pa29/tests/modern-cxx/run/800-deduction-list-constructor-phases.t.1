typedef decltype(sizeof(0)) Size;
namespace std {
template<class T>class initializer_list {
  const T *data; Size count;
public:
  constexpr initializer_list():data(nullptr),count(0) {}
  constexpr const T *begin()const { return data; }
  constexpr Size size()const { return count; }
};
}
template<class T>struct Range {
  int tag;
  Range(const T*,const T*):tag(1) {}
  Range(std::initializer_list<T>):tag(2) {}
};
template<class,class>struct Same { static constexpr bool value=false; };
template<class T>struct Same<T,T> { static constexpr bool value=true; };
template<class T=int>struct Box {
  int tag;
  Box():tag(0) {}
  Box(std::initializer_list<T>):tag(2) {}
};
struct Derived:Box<int> {};
int main() {
  int values[2]={7,9}; int *first=values,*last=values+2;
  auto list=Range{first,last}; auto pair=Range(first,last);
  static_assert(Same<decltype(list),Range<int*>>::value,"list phase before pointer range");
  static_assert(Same<decltype(pair),Range<int>>::value,"parenthesized range");
  Box empty{}; Box filled{7,9}; Box copied{filled};
  Derived derived; Box base{derived};
  static_assert(Same<decltype(copied),Box<int>>::value,"copy deduction skips list phase");
  static_assert(Same<decltype(base),Box<int>>::value,"derived copy deduction");
  return list.tag==2 && pair.tag==1 && empty.tag==0 && filled.tag==2 && copied.tag==2 && base.tag==0?0:1;
}
