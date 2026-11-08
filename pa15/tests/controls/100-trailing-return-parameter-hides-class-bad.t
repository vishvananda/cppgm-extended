// expect reject
// N3485 3.3.3: hidden class name cannot rescue a call of an integer parameter.
struct callee{}; template<class T>auto invoke(T callee)->decltype(callee()){return callee();} int main(){invoke(3);}
