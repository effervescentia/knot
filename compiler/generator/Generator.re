open Core;

let generate = (printer, ast) => {
  Printf.sprintf(
    "module.exports=(function(%s){var %s={};",
    module_map,
    export_map,
  )
  |> printer;
  Module.generate(printer, ast);
  Printf.sprintf("return %s;})(%s);", export_map, module_map) |> printer;
};