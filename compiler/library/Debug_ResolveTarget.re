open AST;
open ResolveTarget;
open Debug_Util;
open Debug_AST;

let print_resolve_target =
  fun
  | ModuleScope(module_) => module_ |~> print_ast
  | DeclarationScope(name, decl) => decl |~> print_decl(name)
  | ImportScope(module_, import) => import |~> print_import
  | ExpressionScope(expr) => expr |~> print_expr
  | ParameterScope(p)
  | PropertyScope(p) => p |~> print_property
  | ReferenceScope(refr) => refr |~> print_ref
  | TypeScope(type_) => type_ |~> Printf.sprintf("type(%s)");
