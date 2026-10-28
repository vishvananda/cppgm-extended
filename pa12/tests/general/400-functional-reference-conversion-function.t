int conversions;
struct Proxy {
  unsigned char sentinel;
  bool value;
  operator bool() const { ++conversions; return value; }
};
int main() {
  typedef bool&& Forwarded;
  Proxy no{64, false};
  bool&& first = Forwarded(no);
  no.value = true;
  if (first || conversions != 1) return 1;
  Proxy yes{0, true};
  bool&& second = Forwarded(yes);
  yes.value = false;
  return !second || conversions != 2;
}
