open AST;

type resolve_target =
  | ModuleScope(ast_module)
  | DeclarationScope(string, ast_declaration)
  | ImportScope(string, ast_import_target)
  | ParameterScope(ast_property)
  | PropertyScope(ast_property)
  | StatePropertyScope(string, ast_state_property)
  | StateMethodScope(string, ast_state_method)
  | ExpressionScope(ast_expression)
  | ScopedExpressionScope(ast_scoped_expression)
  | ReferenceScope(ast_reference)
  | TypeScope(ast_type)
  | MixinScope(ast_type);
