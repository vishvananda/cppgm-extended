struct Point {
    int value;
    constexpr Point(int n) : value(n) {}
    static const Point origin;
};
extern const int answer;
#if __cplusplus >= 201703L
inline constexpr Point Point::origin = {19};
inline const int answer = 37;
#endif
