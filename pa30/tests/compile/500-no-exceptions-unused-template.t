template<class T> void unused_throw(T value) { throw value; }
template<class T> void unused_rethrow() { throw; }
template<class T> int unused_expression(T value) { return value ? 1 : throw value; }
template<class T> void unused_try(T value) { try { (void)value; } catch (...) {} }
template<class T> void unused_function_try(T value) try { (void)value; } catch (...) {}
int main() { return 0; }
