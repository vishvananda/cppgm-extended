template<int N> struct Number { typedef int type; static constexpr int value=N; };
template<class T> struct Reader;
class Secret { static const int value = 7; friend struct Reader<int>; };
template<class T> struct Reader {
 static int read() {
   typedef Number<Secret::value+0> Alias;
   typename Alias::type value = Alias::value;
   return value;
 }
};
int main(){return Reader<double>::read()-7;}
