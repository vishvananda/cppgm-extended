// Bounded copies preserve call order, aliased state and the final counter.
struct State { unsigned data, count, trace; };
typedef unsigned (*Operation)(unsigned, State*);

__attribute__((noinline)) unsigned mutate(unsigned value, State* state)
{
  state->data = (state->data * 33u) ^ value;
  ++state->count;
  state->trace = (state->trace ^ value) * 17u;
  return state->data + 7u;
}

template <int First, int Last, bool Descending>
__attribute__((noinline)) unsigned exercise(Operation operation, State* state)
{
  unsigned total = 0;
  int index = First;
  for (; Descending ? index > Last : index < Last; Descending ? --index : ++index) {
    unsigned value = state->data + static_cast<unsigned>(index);
    value ^= value >> 3;
    value += static_cast<unsigned>(index * 17);
    value = value * 13u + 5u;
    value ^= value << 7;
    value += value >> 11;
    total += operation(value, state);
  }
  return total ^ static_cast<unsigned>(index);
}

int main()
{
  State s0 = {0x12345678u, 0, 0};
  if (exercise<-4, -4, false>(&mutate, &s0) != 4294967292u ||
      s0.data != 305419896u || s0.count != 0u || s0.trace != 0u) return 1;
  State s1 = {0x12345678u, 0, 0};
  if (exercise<-3, -2, false>(&mutate, &s1) != 2551510890u ||
      s1.data != 1743456397u || s1.count != 1u || s1.trace != 882314565u) return 1;
  State s2 = {0x12345678u, 0, 0};
  if (exercise<-2, 0, false>(&mutate, &s2) != 1873748474u ||
      s2.data != 130233125u || s2.count != 2u || s2.trace != 3066209789u) return 1;
  State s3 = {0x12345678u, 0, 0};
  if (exercise<-1, 2, false>(&mutate, &s3) != 1370289332u ||
      s3.data != 3393647930u || s3.count != 3u || s3.trace != 4069710674u) return 1;
  State s4 = {0x12345678u, 0, 0};
  if (exercise<0, 4, false>(&mutate, &s4) != 300127489u ||
      s4.data != 1247607277u || s4.count != 4u || s4.trace != 4189306053u) return 1;
  State s5 = {0x12345678u, 0, 0};
  if (exercise<1, 6, false>(&mutate, &s5) != 2019334652u ||
      s5.data != 270415495u || s5.count != 5u || s5.trace != 2741830639u) return 1;
  State s6 = {0x12345678u, 0, 0};
  if (exercise<2, 10, false>(&mutate, &s6) != 1250819180u ||
      s6.data != 3126897488u || s6.count != 8u || s6.trace != 1852791944u) return 1;
  State s7 = {0x12345678u, 0, 0};
  if (exercise<3, 2, false>(&mutate, &s7) != 3u ||
      s7.data != 305419896u || s7.count != 0u || s7.trace != 0u) return 1;
  State s8 = {0x12345678u, 0, 0};
  if (exercise<-4, -4, true>(&mutate, &s8) != 4294967292u ||
      s8.data != 305419896u || s8.count != 0u || s8.trace != 0u) return 1;
  State s9 = {0x12345678u, 0, 0};
  if (exercise<-3, -4, true>(&mutate, &s9) != 2551510888u ||
      s9.data != 1743456397u || s9.count != 1u || s9.trace != 882314565u) return 1;
  State s10 = {0x12345678u, 0, 0};
  if (exercise<-2, -4, true>(&mutate, &s10) != 2421286339u ||
      s10.data != 130165610u || s10.count != 2u || s10.trace != 3065061762u) return 1;
  State s11 = {0x12345678u, 0, 0};
  if (exercise<-1, -4, true>(&mutate, &s11) != 939851475u ||
      s11.data != 1081231405u || s11.count != 3u || s11.trace != 3470039829u) return 1;
  State s12 = {0x12345678u, 0, 0};
  if (exercise<0, -4, true>(&mutate, &s12) != 73940559u ||
      s12.data != 970137455u || s12.count != 4u || s12.trace != 124255015u) return 1;
  State s13 = {0x12345678u, 0, 0};
  if (exercise<1, -4, true>(&mutate, &s13) != 2946688875u ||
      s13.data != 1444892981u || s13.count != 5u || s13.trace != 2058572365u) return 1;
  State s14 = {0x12345678u, 0, 0};
  if (exercise<2, -6, true>(&mutate, &s14) != 1426575603u ||
      s14.data != 1646076570u || s14.count != 8u || s14.trace != 2403971506u) return 1;
  State s15 = {0x12345678u, 0, 0};
  if (exercise<3, 4, true>(&mutate, &s15) != 3u ||
      s15.data != 305419896u || s15.count != 0u || s15.trace != 0u) return 1;
  return 0;
}
