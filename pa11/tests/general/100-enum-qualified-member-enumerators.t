// Unscoped enumerators remain class members when their enum names qualify them.
struct Base {
  enum Mode { Disabled, Enabled };
  enum Other { Different = 7 };
};
typedef Base Derived;
int calls;
Derived object;
Derived& get() { ++calls; return object; }
int main() {
  Derived value;
  int first = get().Mode::Disabled;
  int second = (&value)->Mode::Enabled;
  int third = value.Base::Other::Different;
  return first == 0 && second == 1 && third == 7 && calls == 1 ? 0 : 1;
}
