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
  | DotAccess(reference, string)
  | Execution(reference, list(expression))
and jsx =
  | Element(string, list((string, expression)), list(jsx))
  | Fragment(list(jsx))
  | TextNode(string)
  | EvalNode(expression);

type import_target =
  | MainExport(string)
  | ModuleExport(string)
  | NamedExport(string, option(string));

type property = (string, option(string), option(expression));

type state_prop =
  | Property(property)
  | Mutator(string, list(property), list(expression))
  | Getter(string, list(property), list(expression));

type style_key =
  | ClassKey(string)
  | IdKey(string);

type style_rule = (reference, reference);
type style_rule_set = (style_key, list(style_rule));

type declaration =
  | ConstDecl(string, expression)
  | StateDecl(string, list(property), list(state_prop))
  | ViewDecl(
      string,
      option(string),
      list(string),
      list(property),
      list(expression),
    )
  | FunctionDecl(string, list(property), list(expression))
  | StyleDecl(string, list(property), list(style_rule_set));

type module_ =
  | Statements(list(module_))
  | Import(string, list(import_target))
  | Declaration(declaration);
