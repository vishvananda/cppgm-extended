// A runtime pure-virtual handler does not take its slot's source parameters.
struct Missing;
struct Abstract {
  Abstract();
  virtual Missing first(Missing) = 0;
  virtual void second(Missing) = 0;
};
Abstract::Abstract() {}
int main() { return 0; }
