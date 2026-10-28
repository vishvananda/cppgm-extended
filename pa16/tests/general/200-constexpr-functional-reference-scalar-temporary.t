typedef const int& Reference;
constexpr int result = Reference(7.5);
static_assert(result == 7, "functional reference cast creates the converted temporary");
int main() { return result != 7; }
