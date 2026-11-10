// C++14 aggregates use omitted default member initializers in static storage.
struct Costs { unsigned a = ~0U, b = ~0U, c = ~0U, d = ~0U; };
struct Entry { int opcode; Costs cost; };
static const Entry table[] = {{7, {1}}, {8, {2, 6, 1, 2}}};
int main() {
 Costs local{1};
 if (local.a != 1 || local.d != ~0U) return 1;
 if (table[0].cost.a != 1 || table[0].cost.b != ~0U || table[0].cost.c != ~0U || table[0].cost.d != ~0U) return 2;
 if (table[1].cost.a != 2 || table[1].cost.b != 6 || table[1].cost.c != 1 || table[1].cost.d != 2) return 3;
 static const Costs tail[3] = {{7}};
 if (tail[0].a != 7 || tail[0].d != ~0U || tail[1].a != ~0U || tail[2].d != ~0U) return 4;
 return 0;
}
