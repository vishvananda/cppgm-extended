#include <algorithm>

using namespace std;

int src[2] = {1, 2};
int dst[2];

int main()
{
  copy_n(src, 2, dst);
  return dst[1] - 2;
}
