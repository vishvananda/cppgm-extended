#include <cstdio>
#include <cstring>
extern "C" long macro_default(long);
extern "C" long macro_keyword(long);
extern "C" long macro_words[10];
extern "C" unsigned char macro_digits[3];
extern "C" char macro_message[];
extern "C" char macro_strings[];
extern "C" char macro_default_string[];
int main() {
    const long expected[] = {11,13,17,3,5,7,19,19,19,19};
    for (unsigned i=0;i<10;++i) if (macro_words[i]!=expected[i]) return 1;
    for (long x=-57;x<58;++x) {
        if (macro_default(x)!=x+11 || macro_keyword(x)!=x+7) return 2;
    }
    if (macro_digits[0]!=2 || macro_digits[1]!=4 || macro_digits[2]!=6) return 3;
    if (std::strcmp(macro_message,"hello world")!=0) return 4;
    if (std::memcmp(macro_strings,"abcd",4)!=0 || std::strcmp(macro_default_string,"default")!=0 || macro_default_string[8]!=31) return 5;
    std::puts("ordered module macros, unique labels, arguments, iteration and conditionals passed");
}
