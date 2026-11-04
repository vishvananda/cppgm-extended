#include "200-host-abi.h"
#include <pthread.h>

extern "C" int from_host_throw() { throw 41; }
extern "C" {
  int destruction_count;
}

static void* run_thread(void*) {
  if (shared_tls != 3 || tlv_add(4) != 7 || shared_tls != 7) return reinterpret_cast<void*>(1);
  if (imported_tls(2) != 13 || host_tls != 13 || !touch_lifetime()) return reinterpret_cast<void*>(2);
  return nullptr;
}

int main() {
  if (init_state != 9) return 1;
  try { from_cppgm_throw(); return 2; }
  catch (int value) { if (value != 29) return 3; }
  if (catch_from_host() != 42) return 4;
  if (shared_tls != 3 || tlv_fp(1.5, 2) != 9.5 || shared_tls != 5) return 5;
  if (tlv_x87(2.25L) != 7.25L) return 6;
  pthread_t thread;
  if (pthread_create(&thread, nullptr, run_thread, nullptr) != 0) return 7;
  void* result = nullptr;
  if (pthread_join(thread, &result) != 0 || result != nullptr || shared_tls != 5) return 8;
  if (destruction_count != 1 || host_tls != 11) return 13;
  Mixed mixed = {7, 1.25};
  mixed = aggregate(mixed);
  if (mixed.integer != 9 || mixed.real != 3.75) return 9;
  NativeBase* value = make_value(23);
  if (value->get() != 23) return 10;
  delete value;
  if (weak_object() != &coalesced() || weak_function(2) != adjust<5>(2)) return 11;
  if (raw_label(4) != 8) return 12;
  return 0;
}
