struct Color { int red, green, blue; };
struct Constants {
  static constexpr int number = 7;
  static constexpr Color color = {2, 3, 5};
  static constexpr char text[] = "abc";
};
constexpr int Constants::number;
template<class T> struct Holder { static constexpr T value = 11; };
const int* other_number();
const Color* other_color();
const char* other_text();
const int* other_template();
