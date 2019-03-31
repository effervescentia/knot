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
  | Array_t(t) => print_member_type(t) |> Printf.sprintf("%s[]")
  | Function_t(args, ret)
  | Mutator_t(args, ret)
  | View_t(args, ret) =>
    Printf.sprintf(
      "(%s) -> %s",
      Util.print_comma_separated(print_member_type, args),
      print_member_type(ret),
    )
  | State_t(params, props) =>
    Printf.sprintf(
      "(%s) -> {%s}",
      Util.print_comma_separated(print_member_type, params),
      print_members(props),
    )
  | Object_t(members) => print_members(members) |> Printf.sprintf("{%s}")
  | Module_t(_, members, main_export) =>
    print_members(members)
    |> (
      s =>
        Printf.sprintf(
          "{%s%s}",
          s,
          switch (main_export) {
          | Some(t) =>
            print_member_type(t) |> Printf.sprintf("\n\t$main: %s\n")
          | None => ""
          },
        )
    )
  | _ => raise(DebugTypeNotSupported)
and print_members = members =>
  Hashtbl.fold(
    (key, value, acc) =>
      (acc == "" ? "\n" : acc)
      ++ Printf.sprintf("\t%s: %s\n", key, print_member_type(value)),
    members,
    "",
  );
