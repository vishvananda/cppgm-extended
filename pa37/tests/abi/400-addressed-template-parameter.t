template<class T> struct Parameter { T value; };
struct Box { int get(Parameter<int>); };
int (Box::*method)(Parameter<int>)=&Box::get;
extern "C" int observe() { return method ? 17 : 0; }
