struct Buffer {
 using Array=int[4];
 Array values;
 constexpr operator const Array&()const{return values;}
};
constexpr Buffer b{{1,2,3,4}};
constexpr const int* pointer=b;
static_assert(pointer==b.values&&pointer[2]==3,"array reference decay");
constexpr bool present=b;
static_assert(present,"pointer boolean suffix");
static_assert(b+2==b.values+2,"converted builtin operand");
constexpr int target(int x){return x+7;}
struct Callable{using Function=int(int);constexpr operator Function&()const{return target;}};
constexpr Callable f{};
constexpr Callable::Function* fp=f;
static_assert(fp==target&&fp(5)==12,"function reference decay");
int main(){return pointer[3]==4&&fp(1)==8?0:1;}
