struct Pair { int a; double b; };
struct __attribute__((packed)) Packed { unsigned char a; long long b; };
struct Empty {};
struct alignas(32) Wide { long long words[3]; };
extern "C" int aggregate_sum(int, ...);
extern "C" int observe() {
 Pair pair = {3, 2.5}; Packed packed = {4, 9}; Wide wide = {{11, 13, 17}};
 return aggregate_sum(7, pair, packed, wide, 0x1.0000000000000000000000001p0L, 19, Empty{}) == 89 ? 17 : 1;
}
