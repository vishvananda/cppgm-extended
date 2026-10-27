// The outer single linkage declaration does not make local variables extern.
extern "C" void function() {
  int& missing_initializer;
}
