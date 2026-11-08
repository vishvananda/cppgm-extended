// expect exit 0
template<class T> struct Store { static constexpr T object = T{7}; };
int main() { struct Local { int value; }; return Store<Local>::object.value == 7 ? 0 : 1; }
