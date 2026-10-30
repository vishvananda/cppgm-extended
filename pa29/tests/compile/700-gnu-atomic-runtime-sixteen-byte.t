struct Value { unsigned long long first, second; };
int main(){Value object={1,2},out={0,0},desired={3,4};
__atomic_load(&object,&out,__ATOMIC_ACQUIRE);
if(out.first!=1||out.second!=2)return 1;
__atomic_store(&object,&desired,__ATOMIC_RELEASE);
__atomic_exchange(&object,&out,&desired,__ATOMIC_ACQ_REL);
if(desired.first!=3||desired.second!=4)return 2;
Value expected={9,9};
if(__atomic_compare_exchange(&object,&expected,&desired,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED))return 3;
if(expected.first!=1||expected.second!=2)return 4;
if(!__atomic_compare_exchange(&object,&expected,&desired,true,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED))return 5;
return object.first!=3||object.second!=4;}
