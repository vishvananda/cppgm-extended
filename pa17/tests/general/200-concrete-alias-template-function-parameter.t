struct Box{using value_type=int;};
template<class T>using Alias=T;
int target(int n){return n+1;}
int main(){return((int(*)(Alias<Box>::value_type))target)(41)!=42;}
