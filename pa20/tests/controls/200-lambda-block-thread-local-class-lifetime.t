// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
int made,destroyed;
struct Object{int value;Object():value(7){++made;}~Object(){++destroyed;}};
typedef int(*Read)();
Read make(){thread_local Object n;return []{return ++n.value;};}
int main(){Read a=make();Read b=make();return made!=1||destroyed!=0||a()!=8||b()!=9;}
