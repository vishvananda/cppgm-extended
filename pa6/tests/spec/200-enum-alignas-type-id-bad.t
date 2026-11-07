// CWG2354: alignas applies to classes and variables, not enumeration types.
enum alignas(int) E : int { a };
