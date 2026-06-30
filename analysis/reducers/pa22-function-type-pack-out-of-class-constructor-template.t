template<class T>
struct Function;

template<class R, class... Args>
struct Function<R(Args...)> {
  template<class F, class = void>
  Function(F f);
};

template<class R, class... Args>
template<class F, class>
Function<R(Args...)>::Function(F f) {}

int source();
void consume(Function<void(int)>);

int main() {
  consume(source());
  return 0;
}
