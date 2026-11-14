# A callee that stores into its storage while there is room, and otherwise
# grows the storage through an allocation, gives each caller the test and the
# store: the copy is the path that does not allocate, and it calls the callee
# where the growth begins.
declare function @allocate(%n : i64) -> ptr [role=allocate_memory]
declare function @note(%n : i64) -> void [linkage=c, binding=strong, object=note]

function @push(%v : ptr, %x : i64) -> void {
  block ^entry:
    %data = load typed ptr %v
    %sf = index i8 %v, 8
    %size = load typed i64 %sf
    %cf = index i8 %v, 16
    %cap = load typed i64 %cf
    %full = cmp eq i64 %size, %cap
    branch %full, ^grow, ^store
  block ^store:
    %at = index i64 %data, %size
    store typed i64 %x, %at
    %n = binary add i64 %size, 1
    store typed i64 %n, %sf
    %lf = index i8 %v, 24
    %last = load typed i64 %lf
    %more = cmp gt i64 %x, %last
    %top = select i64 %more, %x, %last
    store typed i64 %top, %lf
    %tf = index i8 %v, 32
    %total = load typed i64 %tf
    %sum = binary add i64 %total, %x
    store typed i64 %sum, %tf
    return void
  block ^grow:
    %twice = binary shl i64 %cap, 4
    %bytes = binary add i64 %twice, 64
    %fresh = call ptr @allocate(%bytes)
    store typed ptr %fresh, %v
    %wider = binary shl i64 %cap, 1
    %room = binary add i64 %wider, 8
    store typed i64 %room, %cf
    store typed i64 0, %sf
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    call void @note(%cap)
    return void
}

function @three(%v : ptr, %x : i64) -> void {
  block ^entry:
    call void @push(%v, %x)
    %y = binary add i64 %x, 1
    call void @push(%v, %y)
    %z = binary add i64 %x, 2
    call void @push(%v, %z)
    return void
}
