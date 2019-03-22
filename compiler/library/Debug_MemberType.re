open MemberType;
open Globals;

exception DebugTypeNotSupported;

let rec print_member_type =
  fun
  | Number_t => "number"
  | String_t => "string"
  | Boolean_t => "boolean"
  | JSX_t => "jsx"
  | Nil_t => "nil"
  | Array_t(t) => print_type_ref(t) |> Printf.sprintf("%s[]")
  | Function_t(args, ret)
  | Generic_t(Some(Callable_t(args, ret)))
  | View_t(args, ret) => print_callable(args, ret)
  | Generic_t(None) => "any"
  | Object_t(members)
  | Generic_t(Some(Keyed_t(members))) =>
    print_members(members) |> Printf.sprintf("{%s}")
  | Module_t(_, members, main_export) =>
    print_members(members)
    |> (
      s =>
        Printf.sprintf(
          "{%s%s}",
          s,
          switch (main_export) {
          | Some(t) => print_type_ref(t) |> Printf.sprintf("\n\t$main: %s\n")
          | None => ""
          },
        )
    )
  | _ => raise(DebugTypeNotSupported)
and print_type_ref = t => typeof(t^) |> print_member_type
and print_callable = (arg_types, return_type) =>
  Printf.sprintf(
    "(%s) -> %s",
    Util.print_comma_separated(print_type_ref, arg_types),
    print_type_ref(return_type),
  )
and print_members = members =>
  Hashtbl.fold(
    (key, value, acc) =>
      (acc == "" ? "\n" : acc)
      ++ Printf.sprintf("\t%s: %s\n", key, print_type_ref(value)),
    members,
    "",
  );
