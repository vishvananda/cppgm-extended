target wasm32 1
declare function @first() -> i32
declare function @second() -> i32
export @first "same"
export @second "same"
