class noncopyable {
protected:
  noncopyable() {}
  ~noncopyable() {}
};

class service_maker : private noncopyable {
public:
  void make() const {}
};

struct maker : service_maker {
};

struct context {
  explicit context(const service_maker& initial) {
    initial.make();
  }
};

struct io_context : context {
  io_context(const service_maker& initial) : context(initial) {}
};

int main() {
  io_context ioc{maker{}};
  return 0;
}
