// C++11 array qualification: N3485 3.9.3/5, 4.4/4 and 8.5.3.
int main(){int a[2]={1,2};volatile int(&&r)[2]=static_cast<int(&&)[2]>(a);r[1]=5;return a[1]-5;}
