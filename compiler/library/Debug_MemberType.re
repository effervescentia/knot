open MemberType;

exception TypeNotSupported;

let rec print_member_type =
  fun
  | Number_t => "number"
  | String_t => "string"
  | Boolean_t => "boolean"
  | Nil_t => "nil"
  | Array_t(t) => typeof(t^) |> print_member_type |> Printf.sprintf("%s[]")
  | Function_t(arg_types, return_type) =>
    Printf.sprintf(
      "(%s) -> %s",
      "",
      typeof(return_type^) |> print_member_type,
    )
  | _ => raise(TypeNotSupported);
