// An array derivation cannot be the first derivation of a constructor.
struct C {
    (C)[2]();
};
