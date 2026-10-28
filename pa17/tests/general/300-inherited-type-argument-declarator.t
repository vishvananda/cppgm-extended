template<class T> struct Select { typedef T type; };
template<class A, class B> struct Same { static const bool value = false; };
template<class A> struct Same<A, A> { static const bool value = true; };
template<int N> struct Value { static const int value = N; };
template<class T> struct Box {};
template<class T> struct Machine : Select<T>::type {
  template<class S> S get_state() { return this->song; }
};
struct Player {
  struct Playing_ {
    struct slot { int value; };
    slot song;
    enum { mask = 7, three = 3 };
  };
  typedef Machine<Playing_> Playing;
  template<class FSM> static int read(FSM& fsm) {
    return fsm.template get_state<Playing::slot&>().value;
  }
  template<class T> static bool types() {
    return Same<Playing::slot*, T*>::value &&
      Same<Playing::slot const&, T const&>::value &&
      Same<Playing::slot&&, T&&>::value &&
      Same<Playing::slot* const, T* const>::value;
  }
  static_assert(Value<Playing::mask & 3>::value == 3, "bitwise expression");
  static_assert(Value<Playing::mask * 2>::value == 14, "multiplicative expression");
  static_assert(Value<Playing::mask && false>::value == 0, "logical expression");
  static_assert(Value<Playing::mask & Playing_::three>::value == 3, "named operand");
  static_assert(Value<Playing::mask * -2>::value == -14, "unary operand");
  static_assert(Value<Playing::mask & (3)>::value == 3, "parenthesized operand");
  static_assert(Same<Playing::slot* [2], Playing_::slot* [2]>::value, "array of pointers");
  static_assert(Same<Playing::slot* (int), Playing_::slot* (int)>::value, "function type");
  static_assert(Same<Box<Playing::slot>, Box<Playing_::slot>>::value, "bare type name");
};
int main() {
  Player::Playing machine;
  machine.song.value = 7;
  return Player::read(machine) != 7 || !Player::types<Player::Playing_::slot>();
}
