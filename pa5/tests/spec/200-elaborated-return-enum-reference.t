// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect exit 0
enum E{zero}; E value=zero; template<class T>enum E& f(T){return value;} template<>enum E& f<int>(int){return value;} int main(){return f(0);}
