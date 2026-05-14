// VALIDATION: run-pass
// N3485 focus: 14.5.5 [temp.class.spec], 14.5.6 [temp.var]

template<class T, class U>
constexpr int pick_v = 0;

template<class T>
constexpr int pick_v<T, T *> = 1;

int main()
{
  return pick_v<int, int *> == 1 ? 0 : 1;
}
