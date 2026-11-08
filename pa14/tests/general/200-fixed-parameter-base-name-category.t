// A fixed specialization can inherit a name from its template argument.
// Semantic lookup, including hiding, determines whether it is a type or value.
enum class Code { invalid = 19 };
typedef int RecordEvent;
int HiddenType;
struct Root {
  int recorded;
  int Ctx;
  typedef int HiddenType;
  void RecordEvent(Code code) { recorded = static_cast<int>(code); }
};
template<class Current, class Parent = Root> struct Adapter : Parent {};
struct Middle : Adapter<Middle> {};
struct Leaf : Adapter<Leaf, Middle> {
  struct Helper { int value; Helper(int n): value(n) {} };
  Leaf() { RecordEvent(Code::invalid); Ctx = 7; }
  int outside();
  int type_check() {
    HiddenType(item);
    item = 5;
    return item;
  }
  int local_check() {
    int RecordEvent = 3;
    RecordEvent *= 2;
    return RecordEvent;
  }
  int argument_check() { Helper BBGetter(Ctx); return BBGetter.value; }
};
int Leaf::outside() { RecordEvent(Code::invalid); return recorded; }
int main() {
  Leaf value;
  return value.recorded == 19 && value.type_check() == 5
    && value.local_check() == 6 && value.outside() == 19
    && value.argument_check() == 7 ? 0 : 1;
}
