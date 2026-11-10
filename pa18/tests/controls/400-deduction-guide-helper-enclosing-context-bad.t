template<bool B> struct Enable {};
template<> struct Enable<true> { using type = int; };
template<int N> struct Tag {};
template<int N> struct Pick {
  template<class U> static constexpr bool valid() { return N == 1; }
  template<class U, bool Good = valid<U>(), typename Enable<Good>::type = 0>
  Pick(Tag<N>, U) {}
};
int main() {
  Pick first(Tag<1>{}, 7);
  Pick second(Tag<2>{}, 7); // The earlier helper result belongs to Pick<1>.
}
