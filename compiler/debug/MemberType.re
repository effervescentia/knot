open Core;

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
      Knot.Util.print_comma_separated(print_member_type, args),
      print_member_type(ret),
    )
  | Style_t(params, rules) =>
    Printf.sprintf(
      "(%s) -> {%s}",
      Knot.Util.print_comma_separated(print_member_type, params),
      Knot.Util.print_comma_separated(x => x, rules),
    )
  | State_t(params, props) =>
    Printf.sprintf(
      "(%s) -> {%s}",
      Knot.Util.print_comma_separated(print_member_type, params),
      print_members(props),
    )
  | Object_t(members) => print_members(members) |> Printf.sprintf("{%s}")
  | Module_t(_, members, main_export) =>
    print_members(members)
    |> (
      Knot.Util.print_optional(
        print_member_type % Printf.sprintf("\n\t$main: %s\n"),
        main_export,
      )
      |> Printf.sprintf("{%s%s}")
    )
and print_members = members =>
  Hashtbl.fold(
    (key, value, acc) =>
      (acc == "" ? "\n" : acc)
      ++ Printf.sprintf("\t%s: %s\n", key, print_member_type(value)),
    members,
    "",
  );
