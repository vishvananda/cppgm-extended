struct Proxy {
  unsigned char sentinel;
  bool value;
  operator bool() const { return value; }
};
template<class Reference> bool convert(Proxy& proxy) {
  Reference value = Reference(proxy);
  return value;
}
int main() {
  Proxy no{64, false};
  Proxy yes{0, true};
  return convert<bool&&>(no) || !convert<bool&&>(yes);
}
