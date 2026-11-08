// expect exit 0
// N3485 4.4/5.19: qualification-compatible pointer views preserve their stored value.
int object=42; constexpr int* pointer=&object; constexpr const int* const* view=&pointer; constexpr const int* result=*view;
static_assert(result==&object,"qualified pointer-to-pointer value"); int main() { return *result!=42; }
