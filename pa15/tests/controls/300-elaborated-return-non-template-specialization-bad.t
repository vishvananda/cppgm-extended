// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect reject
enum E{zero}; enum E f(int); template<>enum E f<int>(int){return zero;} int main(){}
