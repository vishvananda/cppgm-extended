namespace std {
  template<class T> struct tuple_size;
  template<unsigned long I, class T> struct tuple_element;
}
template<class T> struct Record {
  int value;
  template<unsigned long I> int& get() & { return value; }
};
namespace std {
  template<class T> struct tuple_size<Record<T>> { static const unsigned long value = 1; };
  template<class T> struct tuple_element<0, Record<T>> { typedef int type; };
}
Record<int>& record();
void increment() { auto& [value] = record(); ++value; }
Record<int> object{7};
Record<int>& record() { return object; }
int main() { increment(); return object.value == 8 ? 0 : 1; }
