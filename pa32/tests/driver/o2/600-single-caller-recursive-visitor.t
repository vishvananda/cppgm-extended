// A uniquely called dispatcher can become part of its recursive caller.
struct Span { unsigned begin, end; };
struct Node { unsigned kind, value; const Node* child; };
unsigned visits, dispatches;
static Span route(const Node*, unsigned);
extern "C" Span visit(const Node* node, unsigned depth) {
  if (!node || !depth) return Span{0, 0};
  ++visits;
  return route(node, depth);
}
static Span route(const Node* node, unsigned depth) {
  ++dispatches;
  switch (node->kind) {
  case 0: return Span{node->value, node->value ^ 1u};
  case 1: return Span{node->value + 1u, node->value ^ 3u};
  case 2: return Span{node->value + 2u, node->value ^ 5u};
  case 3: return Span{node->value + 3u, node->value ^ 7u};
  case 4: return Span{node->value + 4u, node->value ^ 9u};
  case 5: return Span{node->value + 5u, node->value ^ 11u};
  case 6: return Span{node->value + 6u, node->value ^ 13u};
  case 7: return Span{node->value + 7u, node->value ^ 15u};
  default: return visit(node->child, depth-1);
  }
}

// A source noinline request survives the same shape of call cycle.
static __attribute__((noinline)) unsigned retained(unsigned);
unsigned retained_peer(unsigned n) { return n ? retained(n-1)+1 : 1; }
static __attribute__((noinline)) unsigned retained(unsigned n) {
  return n ? retained_peer(n-1)+1 : 1;
}

int main() {
  Node nodes[24];
  for (unsigned i=0; i!=24; ++i) {
    nodes[i].kind=i%11;
    nodes[i].value=i*1664525u+1013904223u;
    nodes[i].child=i+1==24 ? 0 : &nodes[i+1];
  }
  for (unsigned i=0; i!=24; ++i) {
    for (unsigned depth=0; depth!=18; ++depth) {
      const Node* p=&nodes[i];
      unsigned remaining=depth, count=0;
      Span expected={0,0};
      while (p && remaining) {
        ++count;
        if (p->kind<8) {
          expected.begin=p->value+p->kind;
          expected.end=p->value^(2*p->kind+1);
          break;
        }
        p=p->child;
        --remaining;
      }
      visits=dispatches=0;
      Span actual=visit(&nodes[i],depth);
      if (actual.begin!=expected.begin || actual.end!=expected.end) return 1;
      if (visits!=count || dispatches!=count) return 2;
      if (retained(depth)!=depth+1 || retained_peer(depth)!=depth+1) return 3;
    }
  }
  Span empty=visit(0,17);
  return empty.begin || empty.end;
}
