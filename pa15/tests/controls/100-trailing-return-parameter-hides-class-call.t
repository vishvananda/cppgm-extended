// expect exit 0
// N3485 3.3.3/3.3.10: prototype parameter name hides enclosing class in trailing return.
struct callee{}; struct Callable{int operator()(){return 7;}}; template<class T>auto invoke(T callee)->decltype(callee()){return callee();} int main(){return invoke(Callable())!=7;}
