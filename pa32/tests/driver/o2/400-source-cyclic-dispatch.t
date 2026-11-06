// Recursive dispatch keeps argument effects, recursion and unwinding intact.
struct Node { int kind, value; const Node* child; const Node* next; };
unsigned observations, depth_sum, fail_calls, argument_throw, cleanups;
volatile unsigned runtime_seed = 13;

struct Guard { ~Guard() { ++cleanups; } };
__attribute__((noreturn,noinline)) void fail(int value) {
  ++fail_calls;
  throw value;
}
unsigned probe(unsigned depth) {
  depth_sum += depth;
  if (argument_throw && depth == 1) throw 77;
  return depth;
}
int walk(const Node*, unsigned);
int descend(const Node* node, unsigned depth) {
  return depth ? walk(node->child, depth-1) : 0;
}
int route(const Node* node, unsigned depth) {
  ++observations;
  switch (node->kind) {
  case 0: return node->value;
  case 1: return -node->value;
  case 2: return descend(node, depth) + node->value;
  default: fail(node->kind);
  }
}
int relay(const Node* node, unsigned depth) { return route(node, probe(depth)); }
int walk(const Node* node, unsigned depth) {
  int total = 0;
  while (node) { total += relay(node, depth); node = node->next; }
  return total;
}

volatile unsigned trace;
unsigned cycle_b(unsigned);
unsigned cycle_a(unsigned n) {
  if (!n) return 1;
  unsigned value = cycle_b(n-1);
  trace = trace * 33u + n;
  return value+1;
}
unsigned cycle_b(unsigned n) {
  if (!n) return 1;
  unsigned value = cycle_a(n-1);
  trace = trace * 33u + n;
  return value+1;
}

unsigned fan_calls;
unsigned fan_b(unsigned);
unsigned fan_a(unsigned n) {
  ++fan_calls;
  return n ? fan_b(n-1)+fan_b(n-1) : 1;
}
unsigned fan_b(unsigned n) {
  ++fan_calls;
  return n ? fan_a(n-1)+fan_a(n-1) : 1;
}

__attribute__((noinline)) unsigned kept(unsigned);
unsigned kept_peer(unsigned n) { return n ? kept(n-1)+1 : 1; }
__attribute__((noinline)) unsigned kept(unsigned n) {
  return n ? kept_peer(n-1)+1 : 1;
}

int main() {
  unsigned state = runtime_seed;
  for (unsigned round=0; round<32; ++round) {
    state=state*1664525u+1013904223u;
    int value=static_cast<int>(state & 31u)-16;
    Node leaf={static_cast<int>(round & 1u), value-2, 0, 0};
    Node root={2, value+3, 0, &leaf}; root.child=&root;
    for (unsigned depth=0; depth<17; ++depth) {
      observations=depth_sum=0;
      int unit=root.value+(leaf.kind ? -leaf.value : leaf.value);
      if (walk(&root,depth)!=static_cast<int>(depth+1)*unit) return 1;
      if (observations!=2*(depth+1) || depth_sum!=depth*(depth+1)) return 2;
    }
    trace=0;
    unsigned expected=0;
    for (unsigned n=1;n<=round;++n) expected=expected*33u+n;
    if (cycle_a(round)!=round+1 || trace!=expected) return 3;
    trace=0;
    if (cycle_b(round)!=round+1 || trace!=expected) return 4;
    unsigned n=round%9;
    fan_calls=0;
    if (fan_a(n)!=(1u<<n) || fan_calls!=(2u<<n)-1) return 5;
    fan_calls=0;
    if (fan_b(n)!=(1u<<n) || fan_calls!=(2u<<n)-1) return 6;
    if (kept(round)!=round+1 || kept_peer(round)!=round+1) return 7;
  }
  Node bad={4,0,0,0};
  observations=fail_calls=cleanups=depth_sum=0;
  try { Guard guard; walk(&bad,1); return 8; }
  catch (int value) { if (value!=4) return 9; }
  if (observations!=1 || fail_calls!=1 || cleanups!=1 || depth_sum!=1) return 10;
  observations=fail_calls=cleanups=depth_sum=0; argument_throw=1;
  try { Guard guard; walk(&bad,1); return 11; }
  catch (int value) { if (value!=77) return 12; }
  if (observations || fail_calls || cleanups!=1 || depth_sum!=1) return 13;
  return 0;
}
