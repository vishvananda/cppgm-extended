#include <nlohmann/json.hpp>
#include <cstdio>
using json=nlohmann::json;
int main(){ json a=json::array(); for(int i=0;i<100000;i++){ json o; o["id"]=i; o["name"]="item "+std::to_string(i); o["v"]={i*0.125,-i,true,nullptr}; o["o"]={{"k","v"},{"d",1.5e3}}; a.push_back(o);} std::string s=a.dump();
 unsigned long h=0; for(int r=0;r<4;r++){ json d=json::parse(s); double t=0; for(auto& e:d) t+=e["v"][0].get<double>(); h+=(unsigned long)t+d.dump().size(); }
 printf("%lu %.20s\n",h,s.c_str()); }
