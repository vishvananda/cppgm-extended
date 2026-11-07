// N3485 7.3.3p7-p8; P1099R5 relaxations start in C++20. expect reject
enum class E{value=7};using E::value;
