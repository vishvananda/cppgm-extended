#include "re2/re2.h"
#include <cstdio>
#include <string>
int main(){ std::string text; for(int i=0;i<400000;i++){ char b[64]; snprintf(b,64,"user%d@host%d.example.org %d ",i,i%91,i*3); text+=b; }
 RE2 re("([a-z]+[0-9]+)@([a-z0-9]+)\\.example\\.org"); re2::StringPiece in(text),a,b; long n=0,h=0; while(RE2::FindAndConsume(&in,re,&a,&b)){n++;h+=a.size()+b.size();}
 std::string t=text.substr(0,2000000); RE2::GlobalReplace(&t,"host([0-9]+)","H\\1"); printf("%ld %ld %zu %d\n",n,h,t.size(),RE2::PartialMatch(text,"host90\\.example\\.org 1199")?1:0); }
