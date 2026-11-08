// expect exit 0
// N3485 8.5.4p5: a list constructor receives an initializer_list view.
typedef decltype(sizeof(0)) Size;
namespace std { template<class T> class initializer_list { const T* data; Size count; public: constexpr initializer_list():data(nullptr),count(0){} constexpr const T* begin()const{return data;} constexpr Size size()const{return count;} }; }
struct Box { int value; constexpr Box(std::initializer_list<int> values):value(*values.begin()) {} };
constexpr Box value{7};
constexpr Box second{11};
static_assert(value.value==7 && second.value==11, "list constructors");
int main() { return value.value != 7; }
