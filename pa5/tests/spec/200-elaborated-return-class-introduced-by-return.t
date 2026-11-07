// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect exit 0
template<class T>struct S* f(T); struct S{int value;}; S value={0}; template<>struct S* f<int>(int){return &value;} int main(){return f(0)->value;}
