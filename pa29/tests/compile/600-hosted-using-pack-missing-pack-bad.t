struct Base { static int choose(int); };
struct Derived : Base { using Base::choose...; };
