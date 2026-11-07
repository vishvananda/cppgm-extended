// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
int main(){int n=7;auto l=[&n]{return ++n;};return l()!=8||l()!=9||n!=9;}
