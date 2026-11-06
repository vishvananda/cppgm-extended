// CXX11-033; N3485 8.4.3p4. expect exit 0
// The primary template's declaration is not an earlier declaration of the explicit specialization.
template<class T> int action() { return 3; }
template<> int action<int>() = delete;
int main() { return action<char>() - 3; }
