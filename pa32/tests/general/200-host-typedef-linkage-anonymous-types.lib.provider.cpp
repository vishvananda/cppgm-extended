typedef struct { int value; } pa32_link_state;
typedef union { int value; long other; } pa32_link_union;
typedef enum { pa32_link_red = 4, pa32_link_blue = 9 } pa32_link_color;

namespace abi_link {
int struct_probe(pa32_link_state *state)
{
  return state->value == 11 ? 0 : 1;
}

int union_probe(pa32_link_union *value)
{
  return value->value == 13 ? 0 : 2;
}

int enum_probe(pa32_link_color color)
{
  return color == pa32_link_blue ? 0 : 4;
}
}
