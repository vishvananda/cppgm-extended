// CWG2354: alignas applies to classes and variables, not enumeration types.
struct alignas(8) C { int n; };
