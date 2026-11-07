// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect reject
int main(){thread_local int n=7;auto l=[&n]{return n;};return l();}
