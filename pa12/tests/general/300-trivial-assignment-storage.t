struct Small { long value; };
struct Pair { long a, b; };
struct alignas(8) Padded { char value; Padded(): value(0) {} };
struct Derived : Padded { char guard; };
struct Bits { unsigned value:3; Bits(): value(0) {} };
struct DerivedBits : Bits { char guard; };
struct Empty {};
struct EmptyDerived : Empty { long value; };
struct Volatile { volatile unsigned value; };
union Union { unsigned long bits; double number; };
int user_calls;
struct User {
 long value;
 User& operator=(const User& x) { ++user_calls; value=x.value+1; return *this; }
};
struct Outer { Small small; User user; Small array[3]; };
int destroyed;
struct Destructible { long value; ~Destructible() { ++destroyed; } };
struct MoveOnly {
 long value;
 MoveOnly& operator=(const MoveOnly&) = delete;
 MoveOnly& operator=(MoveOnly&&) = default;
};
int evaluations;
Small target_value, source_value;
Small& target() { ++evaluations; return target_value; }
const Small& source() { ++evaluations; return source_value; }
Small& assign_small(Small& a, const Small& b) { return a=b; }
Small& explicit_small(Small& a, const Small& b) { return a.operator=(b); }
Pair& assign_pair(Pair& a, const Pair& b) { return a=b; }
Volatile& volatile_assignment(Volatile& a, const Volatile& b) { return a=b; }
User& user_assignment(User& a, const User& b) { return a=b; }
int main() {
 Small a={0}, b={41};
 if (&assign_small(a,b)!=&a || a.value!=41) return 1;
 a.value=0;
 if (&explicit_small(a,b)!=&a || a.value!=41) return 2;
 a.value=17;
 if (&assign_small(a,a)!=&a || a.value!=17) return 3;
 Pair p={0,0}, q={42,43};
 if (&assign_pair(p,q)!=&p || p.a!=42 || p.b!=43) return 4;
 source_value.value=44;
 Small& r=(target()=source());
 if (&r!=&target_value || target_value.value!=44 || evaluations!=2) return 5;
 Small& r2=target().operator=(source());
 if (&r2!=&target_value || evaluations!=4) return 6;
 Derived d,e; d.value=1; d.guard=77; e.value=2; e.guard=88;
 static_cast<Padded&>(d)=static_cast<const Padded&>(e);
 if (d.value!=2 || d.guard!=77) return 7;
 DerivedBits db,eb; db.value=1; db.guard=79; eb.value=5; eb.guard=89;
 static_cast<Bits&>(db)=static_cast<const Bits&>(eb);
 if (db.value!=5 || db.guard!=79) return 8;
 EmptyDerived ed,es; ed.value=81; es.value=91;
 static_cast<Empty&>(ed)=static_cast<const Empty&>(es);
 if (ed.value!=81) return 9;
 Volatile va={0},vb={0x80000001u}; volatile_assignment(va,vb);
 if (va.value!=0x80000001u) return 10;
 Union ua,ub; ua.bits=0;ub.bits=0xfedcba9876543210UL;ua=ub;
 if(ua.bits!=ub.bits) return 11;
 Outer x={{0},{0},{{0},{0},{0}}},y={{45},{46},{{47},{48},{49}}}; x=y;
 if(x.small.value!=45 || x.user.value!=47 || user_calls!=1 || x.array[0].value!=47 || x.array[1].value!=48 || x.array[2].value!=49) return 12;
 { Destructible da={0},ds={51}; da=ds; if(da.value!=51 || destroyed!=0) return 13; }
 if(destroyed!=2) return 14;
 MoveOnly ma={0},mb={52}; ma=static_cast<MoveOnly&&>(mb);
 if(ma.value!=52) return 15;
 return 0;
}
