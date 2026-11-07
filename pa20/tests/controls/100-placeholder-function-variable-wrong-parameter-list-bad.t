// CWG2476 initializer-based function return placeholder deduction; expect reject
int target(int){return 0;} int main(){auto (*p)(long)=target;}
