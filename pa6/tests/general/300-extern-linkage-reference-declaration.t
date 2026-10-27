// N3485 7.5/7: a single declaration in a linkage-specification is extern.
extern "C" int& c_reference;
extern "C" int&& c_rvalue_reference;
extern "C++" int& cpp_reference;
extern "C++" int&& cpp_rvalue_reference;
namespace library {
extern "C" const int& const_reference;
extern int& explicit_reference;
}
