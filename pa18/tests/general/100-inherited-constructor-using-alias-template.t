struct YTraits {};

template<class T>
struct YBase {
  explicit YBase(const T&) {}
};

template<class T>
struct YDerived : public YBase<T> {
  typedef YBase<T> YAlias;
  using YAlias::YAlias;
};

template<class T>
struct YHolder {
  typedef YDerived<T> YMember;

  explicit YHolder(const T& t) : member(t) {}

  YMember member;
};

int main() {
  YTraits t;
  YHolder<YTraits> h(t);
  return 0;
}
