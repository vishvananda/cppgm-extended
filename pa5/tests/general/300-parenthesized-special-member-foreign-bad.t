// An unqualified constructor name must be the current class name.
struct Other;
struct C {
    (Other)();
};
