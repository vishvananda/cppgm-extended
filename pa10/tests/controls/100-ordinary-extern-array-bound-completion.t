// expect exit 0
int arr[3]={4,5,6};int main(){extern int arr[3];extern int arr[];return sizeof(arr)!=3*sizeof(int) || arr[1]!=5;}
