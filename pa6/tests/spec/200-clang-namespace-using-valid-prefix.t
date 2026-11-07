// Independent valid namespace prefix from Clang namespace.udecl/p10.cpp.
// N3485 7.3.3: namespace using-declarations may name compatible functions and class tags.
namespace test0 {
  namespace ns0 {
    class tag;
    int tag();
  }

  namespace ns1 {
    using ns0::tag;
  }

  namespace ns2 {
    using ns0::tag;
  }

  using ns1::tag;
  using ns2::tag;
}

// PR 5752
namespace test1 {
  namespace ns {
    void foo();
  }

  using ns::foo;
  void foo(int);

  namespace ns {
    using test1::foo;
  }
}

