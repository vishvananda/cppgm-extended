namespace reducer {

template<class Signature>
struct Function;

template<class Result, class... Args>
struct Function<Result(Args...)> {
  Function& operator=(const Function&);
};

template<class Result, class... Args>
Function<Result(Args...)>&
Function<Result(Args...)>::operator=(const Function&) {
  return *this;
}

}  // namespace reducer

struct Payload {};

void assign(reducer::Function<Payload()>& lhs,
            const reducer::Function<Payload()>& rhs) {
  lhs = rhs;
}

int main() {
  reducer::Function<Payload()> lhs;
  reducer::Function<Payload()> rhs;
  assign(lhs, rhs);
  return 0;
}
