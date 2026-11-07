// CWG2354: a lone enum declaration is not an alignas subject.
struct C { alignas(8) enum E : int { a }; };
