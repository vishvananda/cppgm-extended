struct Yes { Yes& operator+=(int); };
struct Narrow { Narrow& operator+=(char); };
struct No {};
template<class T> auto probe(T& value) -> decltype(value += {1000}, char{});
long probe(...);
static_assert(sizeof(probe(*static_cast<Yes*>(nullptr))) == sizeof(char), "valid assignment");
static_assert(sizeof(probe(*static_cast<Narrow*>(nullptr))) == sizeof(long), "narrowing conversion");
static_assert(sizeof(probe(*static_cast<No*>(nullptr))) == sizeof(long), "missing operator");
static_assert(sizeof(probe(*static_cast<const Yes*>(nullptr))) == sizeof(long), "const object mismatch");
int main() {}
