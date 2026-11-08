// Fixed parameter and member-call result types stay fixed inside templates.
struct Value {
  template<class T> int read() const { return sizeof(T); }
};
struct Stack {
  template<class T> T pop() { return T(); }
};
template<class T> int read_free(Stack& stack) {
  const auto& object = stack.pop<Value>();
  return object.read<T>();
}
template<class T> struct Writer {
  int read(Stack& stack) {
    auto object = stack.pop<Value>();
    const auto& alias = object;
    return alias.read<T>();
  }
};
int main() {
  Stack stack;
  Writer<unsigned char> writer;
  return read_free<unsigned char>(stack) == 1 && writer.read(stack) == 1 ? 0 : 1;
}
