template<class... T> int sum(T... values) { return (0 + ... + values); }
template<class... T> bool all(T... values) { return (... && values); }
int main() { return sum()==0 && sum(2,3,4)==9 && all() && !all(true,false)?0:1; }
