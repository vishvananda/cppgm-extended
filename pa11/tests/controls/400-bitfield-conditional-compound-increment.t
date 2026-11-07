// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
int f(bool c){S x={1,2,3},y={3,4,3};int a=((c?x.a:y.a)+=2);int b=++(c?x.a:y.a);int d=(c?x.a:y.a)++;return a!=(c?3:5)||b!=(c?4:6)||d!=(c?4:6)||x.a!=(c?5:1)||y.a!=(c?3:7)||x.b!=2||y.b!=4;}int main(){return f(true)||f(false);}
