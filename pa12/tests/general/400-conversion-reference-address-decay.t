int calls;
struct Buffer {
 using Array=int[4];
 int guard;
 Array values;
 operator Array&(){++calls;return values;}
 operator const Array&()const{++calls;return values;}
};
int* returned(Buffer& b){return b;}
const int* returned(const Buffer& b){return b;}
int argument(int* p,int* expected){return p==expected?0:1;}
int target(int x){return x+7;}
struct Callable {
 using Function=int(int);
 operator Function&()const{++calls;return target;}
};
int main(){
 Buffer b{123,{1,2,3,4}};const Buffer& c=b;
 int* p=b;if(p!=b.values)return 1;
 p=0;p=b;if(p!=b.values)return 2;
 if(returned(b)!=b.values||returned(c)!=b.values)return 3;
 if(argument(b,b.values))return 4;
 const int* cp=c;if(cp!=b.values)return 5;
 const void* vp=b;if(vp!=b.values)return 6;
 bool present=b;if(!present)return 7;
 if(b+2!=b.values+2||b!=b.values||c+3!=b.values+3)return 8;
 Buffer::Array& reference=b;reference[1]=42;if(b.values[1]!=42)return 9;
 Callable fn;Callable::Function* fp=fn;if(fp!=target||fp(5)!=12)return 10;
 bool callable=fn;if(!callable)return 11;
 return calls==14?0:13;
}
