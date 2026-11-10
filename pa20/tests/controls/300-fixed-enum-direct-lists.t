enum class Byte:unsigned char {};
enum Plain:unsigned { Zero=0 };
constexpr Byte maximum{255};
constexpr Byte copy{maximum};
static_assert(static_cast<unsigned>(copy)==255,"same enumeration copies");
static_assert(static_cast<unsigned>(Byte{7})==7,"enum temporary");
struct Holder {
  Byte first{7};
  Byte second;
  Holder():second{9} {}
};
int main() {
  Plain value{3};
  Holder holder;
  Byte *allocated=new Byte{5};
  int result=static_cast<int>(holder.first)+static_cast<int>(holder.second)+static_cast<int>(*allocated)+value;
  delete allocated;
  return result==24?0:1;
}
