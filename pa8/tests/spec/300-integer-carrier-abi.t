declare function @host(%pair : i128 [classes=integer_integer]) -> i128 [result_classes=integer_integer]

function @relay(%pair : i128 [classes=integer_integer]) -> i128 [result_classes=integer_integer] {
  block ^entry:
  %result = call i128 @host(%pair)
  return i128 %result
}

function @indirect(%target : ptr, %pair : i128 [classes=integer_integer]) -> i128 {
  block ^entry:
  %result = call i128 %target(%pair) as (%arg : i128 [classes=integer_integer]) -> i128 [result_classes=integer_integer]
  return i128 %result
}
