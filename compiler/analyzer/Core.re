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

type member_locality =
  | External(string)
  | Module;

type scope_member = {
  type_: member_type,
  locality: member_locality,
};

let analyze_list = analyze => List.map(analyze % await_ctx);
