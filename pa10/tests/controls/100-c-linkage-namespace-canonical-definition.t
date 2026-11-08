// expect exit 0
namespace First { extern "C" int take(int); }
namespace Second { extern "C" int take(int x) { return x + 3; } }
int main() { return First::take(4) == 7 && Second::take(4) == 7 ? 0 : 1; }
