// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
int made,destroyed;
struct Object{int value;Object(int x):value(x){++made;}~Object(){++destroyed;}};
typedef int(*Read)();
Read make(){thread_local const Object& n=Object(7);return []{return n.value;};}
int main(){Read a=make();return a()!=7||make()()!=7||made!=1||destroyed!=0;}
