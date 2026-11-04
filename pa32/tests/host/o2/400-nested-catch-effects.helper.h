struct First { int value; };
struct Second { long value; };
extern unsigned trace;
inline void mark(unsigned value) { trace = trace * 33u + value; }
struct Guard {
  unsigned value;
  explicit Guard(unsigned n) : value(n) { mark(value); }
  ~Guard() { mark(value + 100u); }
};
int unknown(int mode);
