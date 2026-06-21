#include <utility>

int main()
{
  []() {
    struct Local
    {
      int value;
    };
    Local first = {1};
    Local second = {2};
    std::pair<Local *, Local *> pointers(&first, &second);
    (void)pointers;
  }();
  return 0;
}
