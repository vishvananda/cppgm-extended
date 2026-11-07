// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect exit 0
union S{int value;}; S value={0}; template<class T>auto f(T)->union S*{return &value;} template<>auto f<int>(int)->union S*{return &value;} int main(){return f(0)->value;}
