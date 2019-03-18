open Core;

let generate = (printer, to_module_name, ast) =>
  Module.generate(printer, to_module_name, ast);
