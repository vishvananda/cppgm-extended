// expect exit 0
// N3485 3.5p10,8.3.4p3,14.6p8: dependent compatibility waits for substitution.
template<class T>int run(){extern int arr[3];extern T arr;return sizeof(arr)==3*sizeof(int)&&arr[0]==4;}int arr[3]={4,5,6};int main(){return !run<int[]>();}
