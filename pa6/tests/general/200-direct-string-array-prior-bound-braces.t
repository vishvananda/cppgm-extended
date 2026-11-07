// N3485 8.3.4p3,8.5p17: a prior bound is inherited; only a string literal initializes this array form.
extern int a[4]; int a[]={1,2}; extern int a[4]; int main(){return 0;}
