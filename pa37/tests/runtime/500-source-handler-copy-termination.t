namespace std {
 typedef void (*terminate_handler)();
 terminate_handler set_terminate(terminate_handler) throw();
}
extern "C" void _Exit(int);
static void on_terminate() { _Exit(0); }
struct Copy {
 Copy() {}
 Copy(const Copy&) { throw 1; }
};
int main() {
 std::set_terminate(on_terminate);
 try {
  try { throw Copy(); }
  catch (Copy value) { return 2; }
 } catch (int) { return 1; }
 return 3;
}
