open Core;

let generate = (printer, ast) => {
  Printf.sprintf(
    "function(%s,%s){var %s={};",
    module_map,
    util_map,
    export_map,
  )
  |> printer;
  Module.generate(printer, ast);
  Printf.sprintf("return %s;}", export_map) |> printer;
};