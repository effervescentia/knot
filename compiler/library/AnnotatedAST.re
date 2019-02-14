type eventual('a, 'b) =
  | Pending('a)
  | Resolved('a, 'b);

type ctxl('a) = ref(eventual('a, unit));

type a_expression =
  | A_AddExpr(ctxl_expression, ctxl_expression)
  | A_SubExpr(ctxl_expression, ctxl_expression)
  | A_MulExpr(ctxl_expression, ctxl_expression)
  | A_DivExpr(ctxl_expression, ctxl_expression)
  | A_LTExpr(ctxl_expression, ctxl_expression)
  | A_GTExpr(ctxl_expression, ctxl_expression)
  | A_LTEExpr(ctxl_expression, ctxl_expression)
  | A_GTEExpr(ctxl_expression, ctxl_expression)
  | A_AndExpr(ctxl_expression, ctxl_expression)
  | A_OrExpr(ctxl_expression, ctxl_expression)
  | A_Reference(ctxl_reference)
  | A_JSX(ctxl_jsx)
  | A_NumericLit(int)
  | A_BooleanLit(bool)
  | A_StringLit(string)
and a_reference =
  | A_Variable(string)
  | A_DotAccess(ctxl_reference, ctxl_reference)
  | A_Execution(ctxl_reference, list(ctxl_expression))
and a_jsx =
  | A_Element(string, list((string, ctxl_expression)), list(ctxl_jsx))
  | A_Fragment(list(ctxl_jsx))
  | A_TextNode(string)
  | A_EvalNode(ctxl_expression)
and ctxl_expression = ctxl(a_expression)
and ctxl_reference = ctxl(a_reference)
and ctxl_jsx = ctxl(a_jsx);

type a_property = (string, option(string), option(ctxl_expression))
and ctxl_property = ctxl(a_property);

type a_state_prop =
  | A_Property(a_property)
  | A_Mutator(string, list(ctxl_property), list(ctxl_expression))
  | A_Getter(string, list(ctxl_property), list(ctxl_expression))
and ctxl_state_prop = ctxl(a_state_prop);

type a_style_rule = (ctxl_reference, ctxl_reference)
and ctxl_style_rule = ctxl(a_style_rule);
type a_style_rule_set = (ctxl(AST.style_key), list(ctxl_style_rule))
and ctxl_style_rule_set = ctxl(a_style_rule_set);

type a_declaration =
  | A_ConstDecl(string, ctxl_expression)
  | A_StateDecl(string, list(ctxl_property), list(ctxl_state_prop))
  | A_ViewDecl(
      string,
      option(string),
      list(string),
      list(ctxl_property),
      list(ctxl_expression),
    )
  | A_FunctionDecl(string, list(ctxl_property), list(ctxl_expression))
  | A_StyleDecl(string, list(ctxl_property), list(ctxl_style_rule_set))
and ctxl_declaration = ctxl(a_declaration);

type a_module =
  | A_Statements(list(ctxl_module))
  | A_Import(string, list(ctxl(AST.import_target)))
  | A_Declaration(ctxl_declaration)
and ctxl_module = ctxl(a_module);
