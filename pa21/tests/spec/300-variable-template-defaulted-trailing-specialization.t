// VALIDATION: run-pass
// N3485 focus: 14.5.6 [temp.var], 14.5.5 [temp.class.spec]

template<class A, class T, class = void>
const bool pair_v = false;

template<class A, class T>
const bool pair_v<A, T*, void> = true;

int main()
{
  return pair_v<int, int*> ? 0 : 1;
}
