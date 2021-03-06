open AST;
open Globals;
open ResolveTarget;
open Debug_Util;
open Debug_AST;

let print_resolve_target =
  fun
  | ModuleScope(module_) => module_ |~> print_ast
  | DeclarationScope(name, decl) => decl |~> print_decl(name)
  | ImportScope(module_, import) =>
    import
    |~> print_import_target
    % (s => Printf.sprintf("%s FROM %s", s, module_))
  | ExpressionScope(expr) => expr |~> print_expr
  | ScopedExpressionScope(expr) => expr |~> print_scoped_expr
  | ParameterScope(p)
  | PropertyScope(p) => p |~> print_property
  | StatePropertyScope(name, p) => p |~> print_state_property(name)
  | StateMethodScope(name, m) => m |~> print_state_method(name)
  | ReferenceScope(refr) => refr |~> print_ref
  | TypeScope(type_)
  | MixinScope(type_) => type_ |~> Printf.sprintf("type(%s)");
