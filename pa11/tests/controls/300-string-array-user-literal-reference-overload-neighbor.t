// expect exit 0
// N3485 8.5.2 versus2.14.8: a literal-operator call is not a string literal; reference binding keeps its value.
typedef const char A[3]; A source={'x','y',0}; A& operator""_s(const char*,unsigned long){return source;} int f(const char(&)[3]){return 1;} int f(const char(&)[5]){return 2;} int main(){return f({"ab"_s})!=1;}
