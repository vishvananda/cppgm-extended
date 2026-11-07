// Clang SemaCXX/storage-class.cpp; N3485 7.1.1p1 requires a nonempty init-declarator-list.
extern struct ExternalType { int member; };
