// VALIDATION: compile-fail
// N3485 focus: 14.1 [temp.param], 14.6.1 [temp.local]

template<typename T>
struct box
{
  typedef int T;
};

int main()
{
  return 0;
}
