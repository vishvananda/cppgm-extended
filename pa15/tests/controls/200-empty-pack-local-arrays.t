// N3485 8.5p7,p16,14.5.3: each array element is value-initialized; expect exit 0
template<class T,class... A> int check(A... a){T value(a...);for(int i=0;i<3;++i)if(value[i]!=0)return 1;return 0;}
typedef int Row[3];typedef volatile int VolatileRow[3];
int main(){return check<Row>()||check<VolatileRow>();}
