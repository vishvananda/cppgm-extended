template<class T> struct Buffer {
 using Array=T[4];
 Array values;
 operator Array&(){return values;}
 operator const Array&()const{return values;}
};
template<class B> int* pointer(B& b){return b;}
template<class B> const int* pointer(const B& b){return b;}
int main(){Buffer<int> b{{1,2,3,4}};const Buffer<int>& c=b;
 if(pointer(b)!=b.values||pointer(c)!=b.values||b+3!=b.values+3)return 1;
 pointer(b)[2]=42;return b.values[2]==42?0:2;}
