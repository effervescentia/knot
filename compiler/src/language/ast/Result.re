/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;
open Module;

include Factory.Make({
  type type_t = Type.t;
});

/* tag helpers */

let of_main_import = x => MainImport(x);
let of_named_import = ((x, y)) => NamedImport(x, y);

let of_main_export = x => MainExport(x);
let of_named_export = x => NamedExport(x);

let of_const = x => Constant(x);
let of_enum = variants => Enumerated(variants);
let of_func = ((args, expr)) => Function(args, expr);
let of_view = ((props, mixins, expr)) => View(props, mixins, expr);

let of_standard_import = imports => StandardImport(imports);
let of_import = ((namespace, imports)) => Import(namespace, imports);
let of_decl = ((name, x)) => Declaration(name, x);
