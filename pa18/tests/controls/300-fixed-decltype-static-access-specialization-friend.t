// expect exit 0
template<class T> struct Select;
class First { static int value; friend struct Select<int>; };
class Second { static int value; friend struct Select<char>; };
template<class T> struct Select {
 template<class U> decltype(First::value) get() { return 3; }
 template<class U> decltype(Second::value) get() { return 7; }
};
int main() { return Select<int>().get<void>() == 3 && Select<char>().get<void>() == 7 ? 0 : 1; }
