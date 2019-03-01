open AST;
open ResolveTarget;
open Debug_Util;
open Debug_AST;

let print_resolve_target =
  fun
  | ModuleScope(module_) => module_ |~> print_ast
  | DeclarationScope(decl) => decl |~> print_decl
  | ImportScope(module_, import) => import |~> print_import
  | ExpressionScope(expr) => expr |~> print_expr
  | ParameterScope(p)
  | PropertyScope(p) => p |~> print_property
  | ReferenceScope(rf) => rf |~> print_ref;