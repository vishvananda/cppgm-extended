// expect reject
template<class T> struct Store { static T object; };
int main() { struct Local { int value; }; Local v = { 7 }; Store<Local>::object = v; }
