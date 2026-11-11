target wasm32 1

function @initialize(%self : ptr, %value : i32) -> void [this_return=yes] {
  block ^entry:
    store i32 %value, %self
    return void
}

function @relay(%self : ptr, %value : i32) -> void [this_return=yes] {
  block ^entry:
    %callee = copy ptr @initialize
    call void %callee(%self, %value) as (%arg0 : ptr, %arg1 : i32) -> void [this_return=yes]
    return void
}

function @observe() -> i32 {
  slot $stored : i32
  block ^entry:
    %address = addr $stored
    call void @relay(%address, 17)
    %result = load i32 $stored
    return i32 %result
}
