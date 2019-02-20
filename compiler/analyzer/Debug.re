open Core;
open Knot.Debug_Util;

include Knot.Debug;

let print_resolve_target =
  fun
  | ModuleScope(module_) => module_ |~> print_a_ast
  | DeclarationScope(decl) => decl |~> print_a_decl
  | ImportScope(import) => import |~> print_import
  | ExpressionScope(expr) => expr |~> print_a_expr
  | ReferenceScope(rf) => rf |~> print_a_ref
  | JSXScope(jsx) => jsx |~> print_a_jsx;
