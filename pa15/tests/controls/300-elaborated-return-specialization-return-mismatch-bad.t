// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect reject
enum E{zero}; enum F{other}; template<class T>enum E f(T); template<>enum F f<int>(int){return other;} int main(){}
