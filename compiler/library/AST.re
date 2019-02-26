open Globals;

type expression =
  | AddExpr(ast_expression, ast_expression)
  | SubExpr(ast_expression, ast_expression)
  | MulExpr(ast_expression, ast_expression)
  | DivExpr(ast_expression, ast_expression)
  | LTExpr(ast_expression, ast_expression)
  | GTExpr(ast_expression, ast_expression)
  | LTEExpr(ast_expression, ast_expression)
  | GTEExpr(ast_expression, ast_expression)
  | AndExpr(ast_expression, ast_expression)
  | OrExpr(ast_expression, ast_expression)
  | Reference(reference)
  | JSX(jsx)
  | NumericLit(int)
  | BooleanLit(bool)
  | StringLit(string)
and reference =
  | Variable(string)
  | DotAccess(reference, string)
  | Execution(reference, list(ast_expression))
and jsx =
  | Element(string, list((string, ast_expression)), list(jsx))
  | Fragment(list(jsx))
  | TextNode(string)
  | EvalNode(ast_expression)
and ast_expression = ctxl_promise(expression);

type import_target =
  | MainExport(string)
  | ModuleExport(string)
  | NamedExport(string, option(string));

type property = (string, option(string), option(ast_expression));

type state_prop =
  | Property(property)
  | Mutator(string, list(property), list(ast_expression))
  | Getter(string, list(property), list(ast_expression));

type style_key =
  | ClassKey(string)
  | IdKey(string);

type style_rule = (reference, reference);
type style_rule_set = (style_key, list(style_rule));

type declaration =
  | ConstDecl(string, ast_expression)
  | StateDecl(string, list(property), list(state_prop))
  | ViewDecl(
      string,
      option(string),
      list(string),
      list(property),
      list(ast_expression),
    )
  | FunctionDecl(string, list(property), list(ast_expression))
  | StyleDecl(string, list(property), list(style_rule_set));

type statement =
  | Import(string, list(import_target))
  | Declaration(declaration)
  | Main(declaration);

type module_ =
  | Module(list(statement));
