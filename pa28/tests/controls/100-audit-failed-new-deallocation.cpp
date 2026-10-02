// AUDIT-ID: EH
// AUDIT-EXPECT: run
// Host-object replacement operators expose the failed-construction obligation.
alignas(16) unsigned char storage[64];
int allocations;
void* operator new(decltype(sizeof(0))) { ++allocations; return storage; }
void operator delete(void*) noexcept { --allocations; }
struct S { S(){throw 7;} };
int main(){
  try { new S(); }
  catch(int value) { return value!=7 || allocations!=0; }
  return 2;
}
