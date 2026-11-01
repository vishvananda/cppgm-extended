#include <fmt/format.h>
#include <cstdio>
#include <string>
int main(){ unsigned long h=0; for(int i=0;i<2000000;i++){ std::string s=fmt::format("{:>8} {:x} {:.3f} {} {:e}",i,i*7,i*0.125,"str",i*1.5f); h=h*31+s.size()+(unsigned char)s[s.size()-1]; }
 fmt::print("{} {:08.3f} {:<6}|{:^7}|\n",h,3.14159,"ab",42); }
