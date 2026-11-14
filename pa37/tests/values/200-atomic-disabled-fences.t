#ifdef __wasm_atomics__
#error Disabling atomics must also clear its target macro.
#endif
extern "C" int observe() {
  __atomic_signal_fence(__ATOMIC_SEQ_CST);
  __atomic_thread_fence(__ATOMIC_RELAXED);
  return 17;
}
