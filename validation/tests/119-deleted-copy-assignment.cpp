struct MoveOnly
{
  MoveOnly() {}
  MoveOnly(const MoveOnly &) = delete;
  MoveOnly(MoveOnly &&) = default;
  MoveOnly & operator=(const MoveOnly &) = delete;
  MoveOnly & operator=(MoveOnly &&) = default;
};

int main()
{
  MoveOnly a;
  MoveOnly b;
  a = b;
  return 0;
}
