// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
int f(){thread_local int n=7;auto l=[]{return ++n;};return l();}
int main(){return f()!=8||f()!=9;}
