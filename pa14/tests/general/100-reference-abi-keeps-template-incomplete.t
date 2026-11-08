// Reference parameters do not require completion of their referred-to class.
struct Missing;
template<class T> struct Deferred {
  T value;
};
void accept(Deferred<Missing>&) {}
struct Abstract {
  Abstract();
  virtual void consume(Deferred<Missing>&) = 0;
};
Abstract::Abstract() {}
int main() { return 0; }
