include Knot.Globals;
include KnotParse.AST;

let module_map = "$$knot_modules$$";
let export_map = "$$knot_exports$$";

let gen_list = printer => Knot.Util.print_separated(",", printer);

let gen_terminated = (generator, items) =>
  List.fold_left(
    (acc, item) => acc ++ generator(item) |> Printf.sprintf("%s;"),
    "",
    items,
  );