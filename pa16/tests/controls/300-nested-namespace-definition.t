// C++17 nested namespace definitions share lookup and reopening with braces.
namespace outer {
struct Token {};
template<class T> struct Box { static constexpr int value = 23; };
}

namespace outer::middle::inner {
using Item = Box<Token>;
constexpr int first = Item::value;
}

namespace outer { namespace middle { namespace inner {
constexpr int second = first + 19;
} } }

namespace outer::middle::inner {
static_assert(second == 42, "reopened nested namespace");
}

namespace chosen = outer::middle::inner;
static_assert(chosen::second == 42, "qualified nested namespace lookup");
