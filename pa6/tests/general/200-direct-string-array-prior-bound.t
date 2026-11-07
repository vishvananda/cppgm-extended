// N3485 8.3.4p3,8.5p17: a prior bound is inherited; only a string literal initializes this array form.
extern char a[5]; char a[]("ab"); extern char b[5]; char b[]="ab"; int main(){return 0;}
