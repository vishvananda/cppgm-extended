// AUDIT-ID: MEMBER
// AUDIT-EXPECT: run
struct A { int prefix; };
struct B { int value; };
struct C:A,B { int result; int read(){return result;} };
int invoke(B* object,int(B::*member)()){return (object->*member)();}
int main(){C object;object.result=7;B* base=&object;
 int(B::*member)()=static_cast<int(B::*)()>(&C::read);
 return invoke(base,member)!=7;}
