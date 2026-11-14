target wasm32 1
global @present : obj<1x1> = zero
declare global @absent : i8 [binding=weak]
declare function @missing() -> i32 [binding=weak]

function @callback() -> i32 {
  block ^entry:
    return i32 17
}

function @observe() -> i32 {
  block ^entry:
    branch @present, ^function, ^bad
  block ^function:
    branch @callback, ^weak_data, ^bad
  block ^weak_data:
    branch @absent, ^bad, ^weak_function
  block ^weak_function:
    branch @missing, ^bad, ^good
  block ^good:
    return i32 17
  block ^bad:
    return i32 0
}
