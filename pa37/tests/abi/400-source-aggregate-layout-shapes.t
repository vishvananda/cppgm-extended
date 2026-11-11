struct Empty {};
struct Derived : Empty { int value; };
struct WithEmpty { Empty empty; int value; };
struct ArrayOne { short values[1]; };
union SingleUnion { int value; };
union MultipleUnion { int first; int second; };
struct Bits { unsigned value : 32; };
struct __attribute__((packed)) PackedBits { unsigned value : 3; };

Derived derived(Derived value) { value.value += 1; return value; }
WithEmpty with_empty(WithEmpty value) { value.value += 2; return value; }
ArrayOne array_one(ArrayOne value) { value.values[0] += 3; return value; }
SingleUnion single_union(SingleUnion value) { value.value += 4; return value; }
MultipleUnion multiple_union(MultipleUnion value) { value.first += 5; return value; }
Bits bits(Bits value) { value.value += 6; return value; }
PackedBits packed(PackedBits value) { value.value += 1; return value; }

extern "C" int layout_shapes() {
  Derived d; d.value = 1;
  WithEmpty w; w.value = 2;
  ArrayOne a = {{3}};
  SingleUnion s = {4};
  MultipleUnion m = {5};
  Bits b = {6};
  PackedBits p = {3};
  return derived(d).value + with_empty(w).value + array_one(a).values[0]
    + single_union(s).value + multiple_union(m).first + bits(b).value + packed(p).value;
}
