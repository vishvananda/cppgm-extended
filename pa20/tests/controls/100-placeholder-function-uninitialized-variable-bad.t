// CWG2476 initializer-based function return placeholder deduction; expect reject
int main(){auto (*p)(int);}
