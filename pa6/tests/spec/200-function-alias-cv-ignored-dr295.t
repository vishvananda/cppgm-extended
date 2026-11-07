// Adopted CWG295 / N3485 8.3.5p6: cv added on a known function type is ignored.
typedef void Function();
typedef const Function ConstFunction;
typedef volatile Function VolatileFunction;
typedef const volatile Function BothFunction;
ConstFunction* first;
VolatileFunction* second;
BothFunction* third;
