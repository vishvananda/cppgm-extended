// N3485 14.3p1/14.5.7; ordinary type argument kind checks remain required; expect reject
template<class A> struct Box {};
template<class A> using TypeOnly = Box<A>;
TypeOnly<7> invalid;
int main() {}
