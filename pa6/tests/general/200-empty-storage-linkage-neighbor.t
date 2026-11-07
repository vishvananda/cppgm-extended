// N3485 7.5: extern introducing a linkage specification is not a storage-class-specifier.
extern "C" struct LinkageType { int member; };
extern "C" { class LinkageForward; enum LinkageEnum { value }; }
