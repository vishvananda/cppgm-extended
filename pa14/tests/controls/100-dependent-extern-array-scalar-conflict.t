// expect reject
// N3485 3.5p10,8.3.4p3,14.6p8: dependent compatibility waits for substitution.
template<class T>void run(){extern int arr[3];extern T arr;}int main(){run<int>();}
