// Minimized from pa21/tests/general/211-extern-template-operator-function-declaration.t
template<class T>
int operator+(T, T);

extern template int operator+<int>(int, int);
