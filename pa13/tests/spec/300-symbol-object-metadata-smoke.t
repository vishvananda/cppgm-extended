declare global @__external_rtti__int : ptr [binding=strong, object=_ZTIi]
declare function @puts(%s : ptr [pass=decay]) -> i32 [linkage=c, binding=strong, object=puts]

function @main() -> i64 [role=entry, binding=strong, keep_alias=yes] {
  block ^entry:
    %0 = const i64 0
    return i64 %0
}
