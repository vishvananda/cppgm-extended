// C++11 array qualification: N3485 3.9.3/5, 4.4/4 and 8.5.3.
int main(){int a=1;int b=2;int* p[2]={&a,&b};int*volatile(&r)[2]=p;r[1]=&a;return *p[1]-1;}
