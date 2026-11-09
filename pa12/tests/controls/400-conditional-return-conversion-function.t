// N3485 6.6.3p2, 8.5p16: copy-initialize a result through a conversion function.
// expect exit 0
struct Q { void* value; };
int conversions;
struct C {
 Q stored;
 operator Q() const { ++conversions; return stored; }
};
Q convert(bool select, const C& first, const C& second) {
 return select ? first : second;
}
int main() { int value=17; C a={{&value}}, b={{0}};
 return convert(true,a,b).value==&value && convert(false,a,b).value==0 && conversions==2 ? 0 : 1;
}
