// expect exit 0
// N3485 8.5.2 versus2.14.8: a literal-operator call is not a string literal; reference binding keeps its value.
typedef const char A[3]; A source={'x','y',0}; A& operator""_s(const char*,unsigned long){return source;} const char(&r)[3]={"ab"_s}; int main(){return r[1]!='y';}
