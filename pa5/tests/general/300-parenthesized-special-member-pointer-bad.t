// A pointer declarator cannot declare a constructor.
struct C {
    (*C)();
};
