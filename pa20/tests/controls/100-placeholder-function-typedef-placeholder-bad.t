// CWG2476 does not permit typedef deduction; expect reject
typedef auto (*P)(int); int main(){return 0;}
