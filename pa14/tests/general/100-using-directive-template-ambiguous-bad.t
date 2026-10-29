namespace A { template<class T> int select(T) { return 1; } }
namespace B { template<class T> int select(T) { return 2; } }
using namespace A; using namespace B;
int main() { return select<int>(1); }
