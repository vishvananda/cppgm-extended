// expect exit 0
// N3485 3.5p10,8.3.4p3,14.6p8: dependent compatibility waits for substitution.
int arr[3]={4,5,6};template<class T>int run(){extern T arr;extern int arr[3];return sizeof(arr)==3*sizeof(int)&&arr[1]==5;}int main(){return !run<int[]>();}
