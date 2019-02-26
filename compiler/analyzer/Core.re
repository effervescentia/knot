include Knot.Core;

module NestedHashtbl = Knot.NestedHashtbl;

type resolve_target =
  | ModuleScope(ast_module)
  | DeclarationScope(ast_declaration)
  | ImportScope(ast_import_target)
  | ParameterScope(ast_property)
  | PropertyScope(ast_property)
  | ExpressionScope(ast_expression)
  | ReferenceScope(ast_reference);

let analyze_list = analyze => List.map(analyze % await_ctx);
