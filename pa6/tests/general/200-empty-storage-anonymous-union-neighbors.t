// N3485 7.1.1p1,9.5p6: anonymous namespace/global unions retain their storage exception.
static union { int global_value; };
namespace Named { static union { int named_value; }; }
namespace { union { int unnamed_value; }; }
