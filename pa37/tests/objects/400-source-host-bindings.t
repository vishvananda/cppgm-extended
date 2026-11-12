#if !__has_attribute(import_module) || !__has_attribute(__import_name__) || !__has_attribute(export_name)
#error missing target attribute probe
#endif
#if !__has_cpp_attribute(clang::import_module) || !__has_cpp_attribute(clang::export_name)
#error missing scoped target attribute probe
#endif
extern "C" {
int linked_add(int) __attribute__((import_module("host"), import_name("add")));
int linked_add(int) __attribute__((__import_module__("ho" "st"), __import_name__("a" "dd")));
[[clang::import_module("host"), clang::import_name("next")]] int linked_next(int);
int default_field(int) __attribute__((import_module("host")));
int env_alias(int) __attribute__((import_name("add")));
long long linked_wide(long long) __attribute__((import_module("host"), import_name("wide")));
double linked_half(double) __attribute__((import_module("host"), import_name("half")));

[[clang::export_name("answer")]] int answer_impl(int value) { return linked_add(value) + 5; }
[[clang::export_name("counter")]] int counter_impl(int value) { return linked_next(value); }
__attribute__((export_name("defaults"))) int defaults_impl(int value)
{ return default_field(value) + env_alias(value); }
__attribute__((export_name("wide"))) long long wide_impl(long long value) { return linked_wide(value) + 3; }
__attribute__((export_name("half"))) double half_impl(double value) { return linked_half(value); }
__attribute__((export_name("indirect"))) int indirect_impl(int value)
{ int (*volatile selected)(int) = linked_add; return selected(value); }
__attribute__((import_module("host"), import_name("absent"), export_name("defined")))
int local_definition(int value) { return value + 1; }
}
