// N3485 7.1.1p1: mutable requires a declarator, not merely a class-name.
struct Owner { mutable struct MutableType { int member; }; };
