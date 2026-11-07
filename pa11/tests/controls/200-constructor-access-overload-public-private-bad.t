// expect reject
// N3485 11: access belongs to the selected declaration, independent of earlier permitted uses.
struct D { D(int){} private: D(){} }; void good(){D d(1);} void bad(){D d;}
