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
  | Variable(string)
  | NumericLit(int)
  | BooleanLit(bool)
  | StringLit(string);

type import_target =
  | MainExport(string)
  | ModuleExport(string)
  | NamedExport(string, option(string));

type param = (string, option(string), option(expression));

type state_prop =
  | Property(string, option(string), option(expression))
  | Mutator(string, list(param), list(expression))
  | Getter(string, list(param), list(expression));

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
  | FunctionDecl(string, list(param), list(expression));

type module_ =
  | Statements(list(module_))
  | Import(string, list(import_target))
  | Declaration(declaration);