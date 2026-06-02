#include <csignal>
#include <type_traits>

static_assert(SIGTERM > 0, "SIGTERM defined by <csignal>");
static_assert(std::is_same<decltype(std::raise(0)), int>::value, "std::raise(int) -> int");

int call_raise() { return std::raise(SIGTERM); }
int main() { return 0; }
