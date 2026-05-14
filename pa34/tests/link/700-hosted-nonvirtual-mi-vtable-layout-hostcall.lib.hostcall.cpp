struct HostAbiBase
{
  virtual int first();
  virtual int second();
  virtual int third();
};

extern "C" int hosted_vtable_layout_call_third(HostAbiBase * base)
{
  return base->third();
}
