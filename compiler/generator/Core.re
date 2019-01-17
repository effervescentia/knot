include Knot.Globals;
include KnotParse.AST;

let module_map = "$$knot_modules$$";
let util_map = "$$knot_utils$$";
let export_map = "$$knot_exports$$";

let gen_list = printer => Knot.Util.print_separated(",", printer);

let gen_rest = printer =>
  Knot.Util.print_separated("", printer % Printf.sprintf(",%s"));

let gen_terminated = (generator, items) =>
  List.fold_left(
    (acc, item) => acc ++ generator(item) |> Printf.sprintf("%s;"),
    "",
    items,
  );

let gen_string =
  Str.global_replace(Str.regexp_string("'"), "\'") % Printf.sprintf("'%s'");