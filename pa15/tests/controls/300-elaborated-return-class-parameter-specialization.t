// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect exit 0
struct S{int value;}; S value={0}; template<class T>int f(struct S*){return 1;} template<>int f<int>(struct S* p){return p->value;} int main(){return f<int>(&value);}
