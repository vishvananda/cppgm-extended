#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>
template<class T> struct Threshold { static const int value = sizeof(T) < 16; };
unsigned length(const char* text) { return __builtin_strlen(text); }
int main() {
 std::vector<long> values = {9, 3, 5};
 std::sort(values.begin(), values.end());
 std::function<int(int)> increment = [](int n) { return n + 1; };
 struct Local { int member; };
 char text[] = {'w', 'a', 's', 'i', 0};
 if (values[0] != 3 || values[2] != 9 || increment(values[1]) != 6) return 1;
 if (!Threshold<Local>::value || length(text) != 4) return 2;
 volatile float sign = -1.0f;
 if (std::copysign(3.0f, sign) != -3.0f) return 3;
 std::printf("hosted algorithms pass\n");
}
