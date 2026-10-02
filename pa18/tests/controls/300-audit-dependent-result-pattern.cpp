// AUDIT-ID: MANGLE-RESULT
// AUDIT-EXPECT: symbols
template<class T>auto identity(T value)->decltype(value){return value;}
long selected(double){return 7;}
template<class T>auto result(T value)->decltype(selected(value)){return selected(value);}
int (*identity_address)(int)=&identity<int>;
long (*result_address)(int)=&result<int>;
int main(){return identity(7)!=7||result(0)!=7;}
