// expect reject
// N3485 5.19/8.5.4: a constant list carrier does not hide a nonconstant element value read.
typedef decltype(sizeof(0)) Size; namespace std { template<class T> class initializer_list { const T* data; Size count; public: constexpr initializer_list():data(nullptr),count(0) {} constexpr Size size() const { return count; } constexpr const T* begin() const { return data; } }; }
int ordinary=7; constexpr std::initializer_list<int> object={ordinary}; int main() { return 0; }
