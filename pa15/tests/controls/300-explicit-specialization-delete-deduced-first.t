// CXX11-033; N3485 8.4.3p4 and 14.7.3p10. expect exit 0
// An explicitly specialized function may omit its deducible template argument list.
template<class T> int action(T) { return 3; }
template<> int action(int) = delete;
int main() { return action('x') - 3; }
