# Observable writes keep the helper's work when ordinary stores fold.
global @sink : i64 = 0

function @helper(%value : i64) -> i64 [binding=internal] {
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

function @caller(%value : i64) -> i64 [binding=strong] {
  block ^entry:
    %result = call i64 @helper(%value)
    return i64 %result
}
