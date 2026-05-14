// Minimized from pa22/tests/general/330-reference-shell-qualified-storage-type-recursion.t
class S {
protected:
  typedef int iterator;
};

S::iterator *p;
