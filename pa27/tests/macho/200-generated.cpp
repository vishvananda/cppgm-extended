#include "200-host-abi.h"

extern "C" {
  int init_state;
  thread_local int shared_tls = 3;
}

struct Initialize {
  Initialize() { init_state = 9; }
};
Initialize initializer;

extern "C" int from_cppgm_throw() { throw 29; }
extern "C" int catch_from_host() {
  try { return from_host_throw(); }
  catch (int value) { return value + 1; }
}
extern "C" int tlv_add(int value) { shared_tls += value; return shared_tls; }
extern "C" double tlv_fp(double value, int increment) {
  double twice = value * 2;
  return twice + tlv_add(increment) + value;
}
extern "C" long double tlv_x87(long double value) { return value + shared_tls; }
extern "C" Mixed aggregate(Mixed value) { value.integer += 2; value.real *= 3; return value; }
struct NativeValue : NativeBase {
  int value;
  explicit NativeValue(int n) : value(n) {}
  int get() const override { return value; }
};
extern "C" NativeBase* make_value(int value) { return new NativeValue(value); }
extern "C" int* weak_object() { return &coalesced(); }
extern "C" int weak_function(int value) { return adjust<5>(value); }
extern "C" int raw_label(int value) { return value * 2; }
