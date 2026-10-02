// C++11 reference initialization: N3485 8.5.3.
int main(){const int a[2][2]={{1,2},{3,4}};const int(&r)[2][2]=static_cast<const int(&&)[2][2]>(a);return &r[0][0]!=&a[0][0]||r[1][1]!=4;}
