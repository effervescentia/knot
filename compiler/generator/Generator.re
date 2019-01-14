open Core;

let rec generate = (printer, ast) => {
  Printf.sprintf("module.exports=(function(%s){", module_map) |> printer;
  Module.generate(printer, ast);
  Printf.sprintf("})(%s);", module_map) |> printer;
};