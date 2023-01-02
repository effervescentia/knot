/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;
open Module;

include Factory.Make({
  type type_t = Type.t;
});

/* tag helpers */

let of_const = x => Constant(x);
let of_enum = variants => Enumerated(variants);
let of_func = ((args, expr)) => Function(args, expr);
let of_view = ((props, mixins, expr)) => View(props, mixins, expr);

let of_standard_import = imports => StdlibImport(imports);
let of_import = ((namespace, main_import, named_imports)) =>
  Import(namespace, main_import, named_imports);
let of_export = ((kind, name, x)) => Export(kind, name, x);
