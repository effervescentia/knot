include Knot.Core;

type generator_core = {
  to_module_name: string => string,
  to_import_statement:
    (string, option(string), list((string, option(string)))) => string,
  to_export_statement: (string, option(string)) => string,
};

let _local_prop = Printf.sprintf("$_%s");

let globals_map = "$$knot_globals$$";
let module_map = "$$knot_modules$$";
let export_map = "$$knot_exports$$";

let injected_variable = "$$knot_injected$$";
let class_map = "$$knot_classes$$";
let state_map = "$$_state";
let props_map = _local_prop("props");
let state_factory = _local_prop("state_factory");
let mutator_factory = state_factory ++ ".mut";
let property_factory = state_factory ++ ".prop";
let factory_constructor = state_factory ++ ".build";

let platform_plugin = "$$knot_platform$$";
let jsx_plugin = "$$knot_jsx$$";
let style_plugin = "$$knot_style$$";

let main_export = "main";

let gen_list = printer => Knot.Print.sequential(~separator=",", printer);

let gen_rest = printer =>
  Knot.Print.sequential(printer % Printf.sprintf(",%s"));

let gen_terminated = (generator, items) =>
  List.fold_left(
    (acc, item) => acc ++ generator(item) |> Printf.sprintf("%s;"),
    "",
    items,
  );

let gen_string =
  Str.global_replace(Str.regexp_string("'"), "\'") % Printf.sprintf("'%s'");
