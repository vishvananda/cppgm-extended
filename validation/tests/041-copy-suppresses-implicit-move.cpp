struct CopiesOnMove
{
  CopiesOnMove() : copied(0) {}
  CopiesOnMove(const CopiesOnMove &) : copied(1) {}

  int copied;
};

int main()
{
  CopiesOnMove source;
  CopiesOnMove dest(static_cast<CopiesOnMove &&>(source));
  return dest.copied == 1 ? 0 : 1;
}
