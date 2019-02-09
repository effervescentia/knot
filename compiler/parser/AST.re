type expression =
  | AddExpr(expression, expression)
  | SubExpr(expression, expression)
  | MulExpr(expression, expression)
  | DivExpr(expression, expression)
  | LTExpr(expression, expression)
  | GTExpr(expression, expression)
  | LTEExpr(expression, expression)
  | GTEExpr(expression, expression)
  | AndExpr(expression, expression)
  | OrExpr(expression, expression)
  | Reference(reference)
  | JSX(jsx)
  | NumericLit(int)
  | BooleanLit(bool)
  | StringLit(string)
and reference =
  | Variable(string)
  | DotAccess(reference, reference)
  | Execution(reference, list(expression))
and jsx =
  | Element(
      string,
      list(element_tag),
      list((string, expression)),
      list(jsx),
    )
  | Fragment(list(jsx))
  | TextNode(string)
  | EvalNode(expression)
and element_tag =
  | ElementKey(string)
  | ElementClass(string);

type import_target =
  | MainExport(string)
  | ModuleExport(string)
  | NamedExport(string, option(string));

type param = (string, option(string), option(expression));

type state_prop =
  | Property(string, option(string), option(expression))
  | Mutator(string, list(param), list(expression))
  | Getter(string, list(param), list(expression));

type style_key =
  | ClassKey(string)
  | IdKey(string);

type style_rule = (reference, reference);
type style_rule_set = (style_key, list(style_rule));

type declaration =
  | ConstDecl(string, expression)
  | StateDecl(string, list(param), list(state_prop))
  | ViewDecl(
      string,
      option(string),
      list(string),
      list(param),
      list(expression),
    )
  | FunctionDecl(string, list(param), list(expression))
  | StyleDecl(string, list(param), list(style_rule_set));

type module_ =
  | Statements(list(module_))
  | Import(string, list(import_target))
  | Declaration(declaration);
