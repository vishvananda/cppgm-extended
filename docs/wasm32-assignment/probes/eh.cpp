// Compare LLVM legacy and modern Wasm EH selection; no runtime is linked.
extern int may_throw();
extern void cleanup();
struct Guard { ~Guard() { cleanup(); } };
extern "C" int catch_it() {
  try { Guard guard; return may_throw(); }
  catch (int x) { return x; }
}
