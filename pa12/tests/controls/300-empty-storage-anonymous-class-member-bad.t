// expect reject
// N3485 9.5p6: an anonymous union in class scope has no storage-class-specifier.
struct Owner { static union { int member; }; };
