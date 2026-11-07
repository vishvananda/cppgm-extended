// CWG2354: alignas applies to classes and variables, not enumeration types.
enum E : int { a }; alignas(8) E value;
