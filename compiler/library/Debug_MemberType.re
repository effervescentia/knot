open MemberType;
open Globals;

exception TypeNotSupported;

let rec print_member_type =
  fun
  | Number_t => "number"
  | String_t => "string"
  | Boolean_t => "boolean"
  | JSX_t => "jsx"
  | Nil_t => "nil"
  | Array_t(t) => print_type_ref(t) |> Printf.sprintf("%s[]")
  | Function_t(args, ret) => print_callable(args, ret)
  | Generic_t(t) =>
    switch (t) {
    | Some(t) =>
      switch (t) {
      | Callable_t(args, ret) => print_callable(args, ret)
      | Keyed_t(members) => print_members(members) |> Printf.sprintf("{%s}")
      }
    | None => "any"
    }
  | Module_t(_, members, _) =>
    print_members(members) |> Printf.sprintf("module {%s}")
  | _ => raise(TypeNotSupported)
and print_type_ref = t => typeof(t^) |> print_member_type
and print_callable = (arg_types, return_type) =>
  Printf.sprintf(
    "(%s) -> %s",
    Util.print_comma_separated(print_type_ref, arg_types),
    typeof(return_type^) |> print_member_type,
  )
and print_members = members =>
  Hashtbl.fold(
    (key, value, acc) =>
      (acc == "" ? "\n" : acc)
      ++ Printf.sprintf("\t%s: %s\n", key, print_type_ref(value)),
    members,
    "",
  );
