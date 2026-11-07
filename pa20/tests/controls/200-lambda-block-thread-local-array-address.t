// N3485 3.7.2/5.1.2: nonautomatic TLS lookup through lambda bodies; expect exit 0
typedef int*(*Address)();
Address make(){thread_local int n[2]={7,8};return []()->int*{return n;};}
int main(){Address a=make();int*p=a();Address b=make();if(p!=b()||p[1]!=8)return 1;p[0]=9;return b()[0]!=9;}
