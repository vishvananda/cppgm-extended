struct Value { int n; int get(unsigned)const { return n; } void set(unsigned,int v) { n=v; } };
using Getter=int(Value::*)(unsigned)const;
using Setter=void(Value::*)(unsigned,int);
constexpr Getter get_member() { return &Value::get; }
constexpr Getter getter=&Value::get;
constexpr Setter setter=&Value::set;
template<Getter Get,Setter Set> struct Action {
  static int run(Value &value) { (value.*Set)(0,9); return (value.*Get)(0); }
};
template<class,class>struct Same { static constexpr bool value=false; };
template<class T>struct Same<T,T> { static constexpr bool value=true; };
static_assert(Same<Action<getter,setter>,Action<&Value::get,&Value::set>>::value,"canonical member identity");
static_assert(Same<Action<get_member(),setter>,Action<true?getter:nullptr,setter>>::value,"constant expressions");
template<int Value::*Member>struct Data { static int read(Value&value) { return value.*Member; } };
constexpr int Value::*field=&Value::n;
int main() { Value value{7}; return Action<getter,setter>::run(value)==9 && Data<field>::read(value)==9?0:1; }
