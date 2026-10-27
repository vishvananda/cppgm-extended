typedef unsigned long Count;
typedef const int Constant;
typedef int Callback(const int);
typedef int Buffer[3];
typedef int& Reference;
struct Value {};
namespace library {
  typedef unsigned long Count;
  typedef const int Constant;
  typedef int Callback(int);
  typedef int Buffer[3];
  typedef int& Reference;
  typedef ::Value Value;
}
using library::Count;
using library::Count;
using library::Constant;
using library::Callback;
using library::Buffer;
using library::Reference;
using library::Value;
Count count;
Callback callback;
Buffer buffer;
extern Reference reference;
void block() {
  typedef unsigned long Count;
  using library::Count;
  Count local;
}
