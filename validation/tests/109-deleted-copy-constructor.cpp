struct CopyBlocked
{
  CopyBlocked() {}
  CopyBlocked(const CopyBlocked &) = delete;
};

int main()
{
  CopyBlocked x;
  CopyBlocked y = x;
  return 0;
}
