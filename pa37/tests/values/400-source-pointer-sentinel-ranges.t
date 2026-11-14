typedef __UINTPTR_TYPE__ UIntPtr;
struct View {
  const char* data;
  unsigned size;
  View(const char* p, unsigned n) : data(p), size(n) {}
};
View empty_key() {return View(reinterpret_cast<const char*>(~UIntPtr(0)), 0);}
View tomb_key() {return View(reinterpret_cast<const char*>(~UIntPtr(1)), 0);}
bool equal(View a, View b) {
  if(b.data==empty_key().data)return a.data==empty_key().data;
  if(b.data==tomb_key().data)return a.data==tomb_key().data;
  return a.data==b.data && a.size==b.size;
}
extern "C" int observe() {
  View entries[]={empty_key(), empty_key(), View("real",4), tomb_key()};
  int count=0;
  for(auto item:entries)
    if(!equal(item,empty_key()) && !equal(item,tomb_key()))++count;
  return count;
}
extern "C" int classify(int value) {
  const char* p = reinterpret_cast<const char*>(UIntPtr(value));
  if (p == empty_key().data) return 7;
  if (p == tomb_key().data) return 9;
  return 0;
}
