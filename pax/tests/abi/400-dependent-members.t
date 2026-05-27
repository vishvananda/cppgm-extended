case dependent_rebind_other
let-type T template-param 0
let-type U template-param 1
let-arg U_arg type U
let-type Rebind member-template T rebind U_arg
type member Rebind other

case dependent_alias_type_id
let-type T template-param 0
let-expr alias member T yes type
type decltype alias

case dependent_owner_member_template
let-type T template-param 0
let-type U template-param 1
let-arg T_arg type T
let-arg U_arg type U
let-type Owner template ns::Outer T_arg
type member-template Owner Iterator U_arg
