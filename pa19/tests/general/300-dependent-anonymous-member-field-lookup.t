template<class T>
struct Box {
  struct {
    T value;
  };

  void set(T x) { value = x; }
};

static_assert(sizeof(Box<int>) == sizeof(int), "");

int main() { return 0; }
