#include "pugixml.hpp"
#include <cstdio>
#include <sstream>
#include <string>
int main(){ std::string s="<?xml version=\"1.0\"?><root>"; for(int i=0;i<200000;i++){ char b[120]; snprintf(b,sizeof b,"<item id=\"%d\" k=\"v%d\"><a>t%d &amp; x</a><b f=\"%g\"/></item>",i,i%37,i,i*0.25); s+=b;} s+="</root>";
 unsigned long h=0; for(int r=0;r<5;r++){ pugi::xml_document d; pugi::xml_parse_result rs=d.load_buffer(s.data(),s.size()); if(!rs){puts(rs.description());return 1;}
 pugi::xpath_node_set ns=d.select_nodes("/root/item[@k='v3']/a"); h+=ns.size(); double v=d.select_node("/root/item[100]/b").node().attribute("f").as_double(); h+=(unsigned long)v;
 std::ostringstream o; d.save(o); h+=o.str().size(); pugi::xpath_query q("sum(/root/item/b/@f)"); h+=(unsigned long)q.evaluate_number(d);}
 printf("%lu\n",h); }
