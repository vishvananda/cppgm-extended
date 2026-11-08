// Reference parameters do not require completion of their referred-to class.
struct Missing;
template<class T> struct Deferred {
  static_assert(sizeof(T) != 0, "this class must not be instantiated");
};
void accept(Deferred<Missing>&) {}
struct Abstract {
  Abstract();
  virtual void consume(Deferred<Missing>&) = 0;
};
Abstract::Abstract() {}
int main() { return 0; }
