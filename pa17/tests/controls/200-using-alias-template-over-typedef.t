// N3485 7.3.3: an alias template cannot redeclare an ordinary typedef.
// expect reject
typedef int Alias;
namespace library { template<class T> using Alias = int; }
using library::Alias;
