include Knot.Core;

let module_map = "$$knot_modules$$";
let util_map = "$$knot_utils$$";
let export_map = "$$knot_exports$$";
let globals_map = "$$knot_globals$$";
let class_map = "$$knot_classes$$";
let props_map = "$$_props";
let state_map = "$$_state";
let update_handler = "$$_update";
let jsx_plugin = "$$_jsxPlugin";
let style_plugin = "$$_stylePlugin";
let main_export = "main";

let gen_list = printer => Knot.Util.print_sequential(~separator=",", printer);

let gen_rest = printer =>
  Knot.Util.print_sequential(printer % Printf.sprintf(",%s"));

let gen_terminated = (generator, items) =>
  List.fold_left(
    (acc, item) => acc ++ generator(item) |> Printf.sprintf("%s;"),
    "",
    items,
  );

let gen_string =
  Str.global_replace(Str.regexp_string("'"), "\'") % Printf.sprintf("'%s'");
