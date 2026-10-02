// AUDIT-ID: TMPL-ACCESS-SFINAE
// AUDIT-EXPECT: run
class Hidden { typedef int type; };
template<class T> int pick(typename T::type*) { return 1; }
template<class T> int pick(...) { return 2; }
int main(){return pick<Hidden>(0)!=2;}
