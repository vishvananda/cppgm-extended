// VALIDATION: compile-pass
// N3485 focus: 12.2 [class.temporary], 6.4 [stmt.select]

struct Probe
{
  static int live;

  Probe()
  {
    ++live;
  }

  Probe(const Probe &)
  {
    ++live;
  }

  ~Probe()
  {
    --live;
  }
};

int Probe::live = 0;

int truthy(const Probe &)
{
  return 1;
}

int main()
{
  if(truthy(Probe())) {
    return Probe::live;
  }
  return 10;
}
