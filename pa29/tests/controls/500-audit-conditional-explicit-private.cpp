// AUDIT-ID: EXPLICIT-CONTEXT
// AUDIT-EXPECT: reject
// AUDIT-DIALECT: hosted-conditional-explicit
class Condition { constexpr explicit operator bool() const {return true;} };
struct Value { template<class T> explicit(T{}) Value(T){} };
int main(){Value v(Condition{});}
