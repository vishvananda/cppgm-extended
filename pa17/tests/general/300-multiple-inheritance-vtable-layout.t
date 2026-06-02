struct HostAbiBase
{
  virtual int first();
  virtual int second();
  virtual int third();
};

struct HostAbiOther
{
  virtual int other();
};

struct HostAbiDerived : HostAbiBase, HostAbiOther
{
  int first();
  int second();
  int third();
  int other();
};

int HostAbiBase::first()
{
  return 1;
}

int HostAbiBase::second()
{
  return 2;
}

int HostAbiBase::third()
{
  return 3;
}

int HostAbiOther::other()
{
  return 4;
}

int HostAbiDerived::first()
{
  return 10;
}

int HostAbiDerived::second()
{
  return 20;
}

int HostAbiDerived::third()
{
  return 30;
}

int HostAbiDerived::other()
{
  return 40;
}

static HostAbiDerived derived;

extern "C" int hosted_vtable_layout_call_third(HostAbiBase * base);

int cppgm_call_third(HostAbiBase * base)
{
  return base->third();
}

int main()
{
  HostAbiBase * base = &derived;
  if(cppgm_call_third(base) != 30) {
    return 1;
  }
  return hosted_vtable_layout_call_third(base) == 30 ? 0 : 2;
}
