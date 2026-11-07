// CWG2354: a lone enum declaration is not an alignas subject.
alignas(8) enum E : int { a };
