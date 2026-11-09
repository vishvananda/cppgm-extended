// A class instantiation does not implicitly define first-declaration-defaulted
// special members. Do not emit an empty outline before an object needs their
// subobject plans. No object here selects Envelope's constructor.
struct Base {
  int base;
  Base() : base(17) {}
};
struct Member {
  int held;
  Member() : held(29) {}
};
template<class T> struct Envelope : Base {
  Envelope() = default;
  Envelope(const Envelope&) = default;
  Envelope(Envelope&&) = default;
  Envelope& operator=(const Envelope&) = default;
  Envelope& operator=(Envelope&&) = default;
  ~Envelope() = default;
  T member;
  int marker = 43;
};
template struct Envelope<Member>;

// A user-provided definition is instantiated, including one defaulted outside
// its class after its first declaration.
template<class T> struct Written {
  Written() : member() {}
  T member;
};
template struct Written<Member>;
template<class T> struct Outside {
  Outside();
  T member;
};
template<class T> Outside<T>::Outside() = default;
template struct Outside<Member>;

// An implicitly deleted default constructor is not one of the members an
// explicit class instantiation defines (14.7.2p8).
struct NoDefault { NoDefault() = delete; };
template<class T> struct DeletedEnvelope {
  DeletedEnvelope() = default;
  T member;
};
template struct DeletedEnvelope<NoDefault>;

// A default member initializer is not required until the defaulted constructor
// is implicitly defined, even though the class is explicitly instantiated.
template<class T> struct DeferredInitializer {
  DeferredInitializer() = default;
  int member = T::not_present;
};
template struct DeferredInitializer<int>;

int main() { return 0; }
