// expect exit 0
template<class T> struct Store { static T object; };
template<class T> T Store<T>::object = {};
int main() { struct Local { int value; }; Store<Local>::object.value = 7; return Store<Local>::object.value == 7 ? 0 : 1; }
