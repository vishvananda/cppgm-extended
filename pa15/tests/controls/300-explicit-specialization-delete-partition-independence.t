// CXX11-033; N3485 8.4.3p4 and 14.7.3. expect exit 0
// Declarations and deletedness belong to each specialization, not its primary's entire table.
template<class T> int action() { return 3; }
template<> int action<int>();
template<> int action<double>() = delete;
template<> int action<int>();
template<> int action<int>() { return 7; }
int main() { return action<int>() != 7 || action<char>() != 3; }
