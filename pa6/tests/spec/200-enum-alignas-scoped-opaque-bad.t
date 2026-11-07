// CWG2354: alignas applies to classes and variables, not enumeration types.
enum class alignas(8) E;
