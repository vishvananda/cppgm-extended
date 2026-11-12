template<class T>
__attribute__((import_module("host"), import_name("add"))) T receive(T);

template<class T>
__attribute__((export_name("answer"))) T api(T value) { return receive(value) + 2; }

template<class T>
__attribute__((export_name("specialized"))) T explicit_api(T);
template<> int explicit_api<int>(int value) { return value + 7; }

extern "C" int instantiate(int value) { return api(value) + explicit_api(value); }
