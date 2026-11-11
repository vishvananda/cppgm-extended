struct One { int value; };
struct Nested { One value; };
struct Pair { int first, second; };
struct SingleDouble { double value; };
struct Mixed { int first; float second; };
struct Large { long long values[3]; };
struct Empty {};
struct alignas(32) Aligned { int value; };

One one(One value) { value.value += 3; return value; }
Nested nested(Nested value) { value.value.value += 5; return value; }
Pair pair(Pair value, int x) { value.first += x; value.second += value.first; return value; }
SingleDouble single_double(SingleDouble value) { value.value *= 2; return value; }
Mixed mixed(Mixed value) { value.first += 2; value.second *= 2; return value; }
Large large(Large value) { value.values[2] += value.values[0] + value.values[1]; return value; }
Aligned aligned(Aligned value) { value.value += 7; return value; }
Empty empty(Empty) { return Empty{}; }
int ignored_middle(int before, Empty, int after) { return before > after ? before - after : after - before; }

extern "C" int aggregates() {
  One a = {10};
  One b = one(a);
  Nested n = {{7}};
  Pair original = {4, 6};
  Pair (*volatile indirect)(Pair, int) = pair;
  Pair direct = pair(original, 3);
  Pair through_pointer = indirect(original, 2);
  SingleDouble d = {1.5};
  Mixed m = {2, 1.5f};
  Large l = {{11, 13, 17}};
  Aligned c = {8};
  Empty e = empty(Empty{});
  return a.value + b.value + nested(n).value.value
    + original.first + direct.first + direct.second
    + through_pointer.first + through_pointer.second
    + static_cast<int>(single_double(d).value)
    + mixed(m).first + static_cast<int>(mixed(m).second)
    + static_cast<int>(large(l).values[2]) + aligned(c).value
    + ignored_middle(19, e, 8);
}
