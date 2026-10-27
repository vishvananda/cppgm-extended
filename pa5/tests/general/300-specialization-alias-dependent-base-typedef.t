// A concrete specialization's inherited typedef remains a type through
// qualified aliases, using aliases, and typedef chains.
template<class T> struct Base { typedef T type; };
template<class T> struct Derived : Base<T> { typedef int own; };
struct Protocol { typedef Derived<int> Alias; };
using First = Protocol::Alias;
typedef First Second;
template<class T> struct Wrapper : T {};
using Wrapped = Wrapper<Second>;
namespace names { using Export = Second; }
using names::Export;
int main() {
    Protocol::Alias::own direct = 1;
    Protocol::Alias::type value = 2;
    const Protocol::Alias::type& qualified = value;
    First::type& reference = value;
    const Second::type* pointer = &reference;
    const Wrapped::type& wrapped = value;
    Export::type& exported = value;
    return direct != 1 || value != 2 || &qualified != pointer ||
        &wrapped != pointer || &exported != pointer;
}
