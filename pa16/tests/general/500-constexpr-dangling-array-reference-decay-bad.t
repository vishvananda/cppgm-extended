struct Buffer{using Array=int[4];Array values;constexpr operator const Array&()const{return values;}};
constexpr const int* bad=Buffer{{1,2,3,4}};
int main(){return bad[0];}
