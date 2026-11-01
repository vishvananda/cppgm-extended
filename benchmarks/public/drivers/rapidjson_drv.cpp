#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <cstdio>
#include <string>
using namespace rapidjson;
int main(){ std::string s="["; for(int i=0;i<200000;i++){ char b[200]; snprintf(b,sizeof b,"%s{\"id\":%d,\"name\":\"item %d\\n\",\"v\":[%g,%d,true,null],\"o\":{\"k\":\"v\",\"d\":1.5e3}}",i?",":"",i,i,i*0.125,-i); s+=b;} s+="]";
 unsigned long h=0; for(int r=0;r<10;r++){ Document d; d.Parse(s.c_str()); if(d.HasParseError()){puts("err");return 1;} StringBuffer sb; Writer<StringBuffer> w(sb); d.Accept(w); h+=sb.GetSize(); double t=0; for(SizeType i=0;i<d.Size();i++) t+=d[i]["v"][0].GetDouble(); h+=(unsigned long)t; }
 printf("%lu\n",h); }
