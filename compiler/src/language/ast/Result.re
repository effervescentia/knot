/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;
open Module;

include Factory.Make({
  type type_t = Type.t;
});

/* tag helpers */

let of_const = expression => Constant(expression);
let of_enum = variants => Enumerated(variants);
let of_func = ((parameters, body)) => Function(parameters, body);
let of_view = ((parameters, mixins, expr)) =>
  View(parameters, mixins, expr);

let of_stdlib_import = named_imports => StdlibImport(named_imports);
let of_import = ((namespace, main_import, named_imports)) =>
  Import(namespace, main_import, named_imports);
let of_export = ((kind, name, declaration)) =>
  Export(kind, name, declaration);
