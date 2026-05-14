struct YTraits {};

template<class T, bool B>
struct YBase {
  explicit YBase(const T&) {}
};

template<class T, bool B>
struct YDerived : public YBase<T, B> {
  typedef YBase<T, B> YAlias;
  using YAlias::YAlias;
};

template<class T>
struct YHolder {
  typedef YDerived<T, true> YMember;

  explicit YHolder(const T& t) : member(t) {}

  YMember member;
};

int main() {
  YTraits t;
  YHolder<YTraits> h(t);
  return 0;
}
