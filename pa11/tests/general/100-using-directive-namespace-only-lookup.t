// A using-directive considers namespace names, including through a prefix,
// while ordinary functions and classes of the same spelling are ignored.
int index(int);
int direct(int);
namespace library {
namespace index {
  typedef int mask;
  struct Context { int Ctx; void set(int); };
}
namespace branch { typedef int word; }
namespace direct { typedef int bits; }
}
struct branch {};
using namespace library;
namespace selected = index;
namespace typed = branch;
using namespace selected;
using namespace typed;
using namespace direct;
void Context::set(int value) { Ctx = value; }
mask flags = (mask)3;
word number = (word)4;
bits extra = (bits)2;
int main() { Context c; c.set(number); return c.Ctx + flags + extra == 9 ? 0 : 1; }
