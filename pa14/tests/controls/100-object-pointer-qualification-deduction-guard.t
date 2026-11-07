// expect reject
// N3485 4.4 and 14.8.2.1p4: deduction cannot turn int** into const int**.
template<class T>int pick(const T**){return 0;}
int main(){int n=0;int* p=&n;return pick(&p);}
