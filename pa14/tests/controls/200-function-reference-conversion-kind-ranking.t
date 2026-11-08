// expect exit 0
// I13: Adopted CWG1328: conversion to a function reference prefers the same declared reference kind.
int left() { return 1; } int right() { return 2; }
template<class T> struct Choice { operator T&() { return left; } operator T&&() { return right; } };
typedef int Function();
int main() { Choice<Function> choice; Function& l=choice; Function&& r=choice; return l()!=1 || r()!=2; }
