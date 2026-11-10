# Observable writes keep the helper's work when ordinary stores fold.
global @sink : i64 = 0

function @target(%value : i64) -> i64 [binding=weak, unwind=no] {
  block ^entry:
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    return i64 %value
}

function @dead(%value : i64) -> i64 [binding=weak, unwind=no] {
  block ^entry:
    %result = call i64 @target(%value)
    return i64 %result
}

function @main(%value : i64) -> i64 [role=entry, unwind=no] {
  block ^entry:
    %result = call i64 @target(%value)
    return i64 %result
}
