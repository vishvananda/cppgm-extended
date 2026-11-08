// Empty placement destinations can be base subobjects with no storage of
// their own. Initializing them must preserve the enclosing object's value.
typedef unsigned long size_t;
void* operator new(size_t, void* address) noexcept { return address; }
struct Empty {};
struct Bucket : Empty {
  unsigned long key;
  Empty& second() { return *this; }
};
int constructors;
struct Counted {
  Counted() { ++constructors; }
};
struct CountedBucket : Counted { unsigned long key; };
void initialize(Empty* address) { ::new (static_cast<void*>(address)) Empty(); }
void initialize_list(Empty* address) { ::new (address) Empty{}; }
int main() {
  Bucket bucket;
  bucket.key = 0x123456789abcdef0UL;
  ::new (&bucket.second()) Empty();
  if (bucket.key != 0x123456789abcdef0UL) return 1;
  initialize(&bucket.second());
  if (bucket.key != 0x123456789abcdef0UL) return 2;
  initialize_list(&bucket.second());
  if (bucket.key != 0x123456789abcdef0UL) return 3;
  CountedBucket counted;
  counted.key = 0xfedcba9876543210UL;
  ::new (static_cast<Counted*>(&counted)) Counted();
  return counted.key == 0xfedcba9876543210UL && constructors == 2 ? 0 : 4;
}
