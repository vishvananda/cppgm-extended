case lambda_closure_type
let-context make function ns::make
type lambda-closure make 0

case local_class_call_operator
let-context make function ns::make
function local make Local operator-call 0

case member_lambda_call_operator
let-context member_make function ns::C::make
function lambda member_make 0 operator-call int
