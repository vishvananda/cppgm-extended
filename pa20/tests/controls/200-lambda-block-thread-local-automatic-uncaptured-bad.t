// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect reject
int main(){int n=7;auto l=[]{return n;};return l();}
