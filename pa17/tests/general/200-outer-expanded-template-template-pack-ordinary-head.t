// N3485 focus: 14.1p15 / 14.3.3p3 / 14.5.3p4-p5 outer pack-expanded template heads
template<int> struct IntHead {};
template<template<int> class Template> struct Use { Template<3> member; };
Use<IntHead> object;
int main() { return sizeof(object) != 1; }
