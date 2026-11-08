// CXX11-127: scalar alias arguments preserve the course Clang ABI across translation units.
namespace captured {
template<int...> struct Values {};
template<class... T> using Cardinal = Values<sizeof...(T)>;
template<class T> int scalar(Cardinal<T>*) { return 10 + sizeof(T); }
int (*keep_int)(Cardinal<int>*) = &scalar<int>;
int (*keep_char)(Cardinal<char>*) = &scalar<char>;
}
int provide() {
 return captured::scalar<int>(nullptr) + captured::scalar<char>(nullptr);
}
