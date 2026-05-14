int constructed = 0;
int destroyed = 0;

struct Probe {
  Probe(const char *) { ++constructed; }
  ~Probe() { ++destroyed; }
};

int use(const Probe &) { return 1; }

int truthy(const char * value)
{
  return value && use(Probe(value));
}

int main()
{
  return (truthy((const char*)0) == 0 && constructed == 0 && destroyed == 0) ? 0 : 1;
}
