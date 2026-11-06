// CXX11-033; N3485 8.4.3p4 and 14.7.3. expect exit 0
// A deleted specialization of the pointer overload leaves the other primary's specialization independent.
template<class T> int action(T) { return 1; }
template<class T> int action(T*) { return 2; }
template<> int action<int>(int*) = delete;
template<> int action<int>(int) { return 3; }
int main() { return action(1) - 3; }
