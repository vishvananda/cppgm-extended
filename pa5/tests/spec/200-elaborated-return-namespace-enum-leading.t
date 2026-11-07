// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect exit 0
namespace N{enum E{zero};} template<class T>enum N::E f(T){return N::zero;} template<>enum N::E f<int>(int){return N::zero;} int main(){return f(0);}
