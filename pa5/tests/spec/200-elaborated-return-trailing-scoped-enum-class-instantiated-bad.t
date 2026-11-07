// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect reject
enum class E{zero}; template<class T>auto f(T)->enum class E{return E::zero;} int main(){f(0);}
