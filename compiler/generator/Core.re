include Knot.Globals;
include KnotParse.AST;

let module_map = "__knot_modules__";

let gen_string =
  Str.global_replace(Str.regexp_string("'"), "\'") % Printf.sprintf("'%s'");

let gen_list = printer => Knot.Util.print_separated(",", printer);

let gen_terminated = (generator, items) =>
  List.fold_left(
    (acc, item) => acc ++ generator(item) |> Printf.sprintf("%s;"),
    "",
    items,
  );