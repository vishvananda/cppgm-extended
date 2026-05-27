#include <vector>

int main()
{
  std::vector<int> a;
  std::vector<int> b;
  a.insert(a.begin(), b.begin(), b.end());
  return 0;
}
