# host O1 simple inline constructor can be canonicalized away
# split
struct HostSimpleCtor {
  int value;
  HostSimpleCtor() : value(3) {}
};

int main()
{
  HostSimpleCtor value;
  return value.value == 3 ? 0 : 1;
}
