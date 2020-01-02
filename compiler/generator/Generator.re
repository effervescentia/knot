open Core;

let generate = (printer, core, ast) => Module.generate(printer, core, ast);

let generate_es6_import_statement = Module_ES6.generate_import;
let generate_es6_export_statement = Module_ES6.generate_export;

let generate_common_import_statement = Module_Common.generate_import;
let generate_common_export_statement = Module_Common.generate_export;
