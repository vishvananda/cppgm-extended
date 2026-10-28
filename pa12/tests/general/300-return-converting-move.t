struct Value {int value;Value(int n):value(n){}};
struct Result {
 int value,selected;
 Result(const Value& v):value(v.value),selected(2){}
 Result(Value&& v):value(v.value),selected(1){}
};
Value global(45);
Result reference(Value& v){return v;}
Result local(){Value v(42);return v;}
Result parameter(Value v){return v;}
Result parenthesized(){Value v(43);return (v);}
Result global_object(){return global;}
Result constant(){const Value v(47);return v;}
int main(){
 Value v(44);
 Result ordinary(v);
 if(ordinary.selected!=2)return 1;
 Result a=local(),b=parameter(v),c=parenthesized();
 Result d=reference(v),f=global_object(),g=constant();
 return a.value!=42 || b.value!=44 || c.value!=43 || a.selected!=1 || b.selected!=1 || c.selected!=1 || d.selected!=2 || f.selected!=2 || g.selected!=2;
}
