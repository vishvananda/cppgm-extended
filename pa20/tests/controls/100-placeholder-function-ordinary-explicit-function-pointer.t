// Existing explicit function-pointer allocation neighbor; expect exit 0
int target(int n){return n+2;} int main(){auto p=new (int(*)(int))(target);int r=(*p)(3)-5;delete p;return r;}
