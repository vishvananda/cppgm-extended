struct Record { int first; int* pointer; int last; };
constexpr Record value = {3, nullptr, 7};
static_assert(sizeof(Record) == 12, "target object layout");
extern "C" int observe() {
 unsigned words[3] = {};
 __builtin_memcpy(words, &value, 12);
 return words[0] == 3 && words[1] == 0 && words[2] == 7 ? 17 : 2;
}
