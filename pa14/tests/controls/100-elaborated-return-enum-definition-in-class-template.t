// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect exit 0
template<class T>struct A{enum E{zero};}; int main(){return A<int>::zero;}
