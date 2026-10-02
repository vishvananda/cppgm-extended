// C++11 array qualification: N3485 3.9.3/5, 4.4/4 and 8.5.3.
int main(){int a[2]={1,2};int(*p)[2]=&a;volatile int(*q)[2]=p;(*q)[1]=5;return a[1]-5;}
