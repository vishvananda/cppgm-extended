#pragma once

struct Mixed {
  long integer;
  double real;
};

struct NativeBase {
  virtual int get() const = 0;
  virtual ~NativeBase() {}
};

template<int N> int adjust(int value) { return value + N; }

extern "C" {
  extern int init_state;
  extern thread_local int shared_tls;
  extern thread_local int host_tls;
  extern int destruction_count;
  int from_host_throw();
  int from_cppgm_throw();
  int catch_from_host();
  int tlv_add(int);
  int imported_tls(int);
  int touch_lifetime();
  double tlv_fp(double, int);
  long double tlv_x87(long double);
  Mixed aggregate(Mixed);
  NativeBase* make_value(int);
  int* weak_object();
  int weak_function(int);
}

#ifdef __APPLE__
extern "C" int raw_label(int) asm("_cppgm_raw_label");
#else
extern "C" int raw_label(int) asm("cppgm_raw_label");
#endif

inline int& coalesced() { static int value = 12; return value; }
