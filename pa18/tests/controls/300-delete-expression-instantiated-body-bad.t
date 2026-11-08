// expect reject
// I13: A deleted destructor is a hard body error even when the delete operand is null.
struct Bad { ~Bad() = delete; }; template<class T> void destroy(T* pointer) { delete pointer; }
int main() { destroy(static_cast<Bad*>(nullptr)); }
