// AUDIT-ID: LOOKUP-NAMESPACE-MIXED
// AUDIT-EXPECT: reject
namespace a { struct X {}; } namespace b { int X; } using namespace a; using namespace b; X value;
