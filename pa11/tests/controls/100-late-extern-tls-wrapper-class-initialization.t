// N3485 3.5/3.7.2: every declaration retains namespace TLS duration; expect exit 0
int made;struct Object{int value;Object():value(7){++made;}~Object(){--made;}};int f(){extern thread_local Object n;return ++n.value;}thread_local Object n;int main(){return f()!=8||n.value!=8||made!=1||f()!=9;}
