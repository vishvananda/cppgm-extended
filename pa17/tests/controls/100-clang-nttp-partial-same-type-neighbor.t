// expect exit 0
// N3485 14.5.5.1p2: unambiguous matching-type neighbor of Clang temp.class.spec/p9.cpp.
// Parameter types deliberately agree; this does not resolve CWG1647 conversions.
template<char C1,char C2>struct X{static const bool value=0;};
template<char C1>struct X<C1,C1>{static const bool value=1;};
template<int,int,int>struct int_values{static const unsigned value=0;};
template<int C1,int C3>struct int_values<C1,12,C3>{static const unsigned value=1;};
int main(){return X<1,2>::value!=0 || X<1,1>::value!=1 || int_values<256,12,3>::value!=1 || int_values<256,11,3>::value!=0;}
