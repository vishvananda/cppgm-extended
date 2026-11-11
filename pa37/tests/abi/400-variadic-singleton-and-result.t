struct Byte { unsigned char value; };
struct Nested { struct Inner { int value; } inner; };
struct Pointer { int *value; };
struct Real { double value; };
struct Result { long long first, second, third; };
extern "C" Result create(int, ...);
extern "C" int observe() {
 int pointed = 17;
 Result result = create(7, Byte{5}, Byte{9}, Nested{{11}}, Pointer{&pointed}, Real{2.5});
 return result.first == 21 && result.second == 28 && result.third == 5 ? 17 : 1;
}
