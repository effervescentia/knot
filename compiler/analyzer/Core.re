include Knot.Core;

module NestedHashtbl = Knot.NestedHashtbl;

type resolve_target =
  | ModuleScope(ctxl_module)
  | DeclarationScope(ctxl_declaration)
  | ImportScope(ctxl_import)
  | ParameterScope(ctxl_property)
  | PropertyScope(ctxl_property)
  | ExpressionScope(ctxl_expression)
  | ReferenceScope(ctxl_reference)
  | JSXScope(ctxl_jsx);

let analyze_list = analyze => List.map(analyze % await_ctx);
