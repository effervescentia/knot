include Knot.Core;

type resolve_target =
  | ModuleScope(ctxl_module)
  | DeclarationScope(ctxl_declaration)
  | ImportScope(ctxl_import)
  | ExpressionScope(ctxl_expression)
  | ReferenceScope(ctxl_reference)
  | JSXScope(ctxl_jsx);

type member_type =
  | Number_t
  | String_t
  | Boolean_t
  | Function_t
  | View_t
  | State_t
  | Style_t
  | Module_t;

type member_locality =
  | External(string)
  | Module;

type scope_member = {
  type_: member_type,
  locality: member_locality,
};

let analyze_list = analyze => List.map(analyze % await_ctx);
