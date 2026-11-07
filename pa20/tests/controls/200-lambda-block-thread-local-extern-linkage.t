// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
thread_local int before=7;
int f(){extern thread_local int before;auto l=[]{return ++before;};return l();}
thread_local int after=11;
int g(){extern thread_local int after;auto l=[]{return ++after;};return l();}
int main(){return f()!=8||before!=8||g()!=12||after!=12;}
