// CWG2354: alignas applies to classes and variables, not enumeration types.
enum alignas(8) E : int { a };
