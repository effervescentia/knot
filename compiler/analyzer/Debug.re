open Core;
open Knot.Debug_Util;

include Knot.Debug;

let print_resolve_target =
  fun
  | ModuleScope(module_) => module_ |~> print_ast
  | DeclarationScope(decl) => decl |~> print_decl
  | ImportScope(import) => import |~> print_import
  | ExpressionScope(expr) => expr |~> print_expr
  | ParameterScope(p)
  | PropertyScope(p) => p |~> print_property
  | ReferenceScope(rf) => rf |~> print_ref;
