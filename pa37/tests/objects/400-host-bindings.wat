(module
  (global $counter (export "counter") (mut i32) (i32.const 10))
  (global (export "bias") i64 (i64.const 1099511627776))
  (func (export "add") (param i32) (result i32)
    local.get 0 i32.const 37 i32.add)
  (func (export "next") (param i32) (result i32)
    global.get $counter local.get 0 i32.add global.set $counter
    global.get $counter)
  (func (export "wide") (param i64) (result i64)
    local.get 0 i64.const 1099511627776 i64.add)
  (func (export "half") (param f64) (result f64)
    local.get 0 f64.const 0.5 f64.mul)
  (func (export "default_field") (param i32) (result i32)
    local.get 0 i32.const 3 i32.mul))
