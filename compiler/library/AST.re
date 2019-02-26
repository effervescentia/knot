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
  | Reference(ast_reference)
  | JSX(jsx)
  | NumericLit(int)
  | BooleanLit(bool)
  | StringLit(string)
and ast_expression = ctxl_promise(expression)
and reference =
  | Variable(string)
  | DotAccess(ast_reference, string)
  | Execution(ast_reference, list(ast_expression))
and ast_reference = ctxl_promise(reference)
and jsx =
  | Element(string, list((string, ast_expression)), list(jsx))
  | Fragment(list(jsx))
  | TextNode(string)
  | EvalNode(ast_expression);

type import_target =
  | MainExport(string)
  | ModuleExport(string)
  | NamedExport(string, option(string))
and ast_import_target = ctxl_promise(import_target);

type property = (
  string,
  option(ctxl_promise(string)),
  option(ast_expression),
)
and ast_property = ctxl_promise(property);

type state_prop =
  | Property(ast_property)
  | Mutator(string, list(ast_property), list(ast_expression))
  | Getter(string, list(ast_property), list(ast_expression))
and ast_state_prop = ctxl_promise(state_prop);

type style_key =
  | ClassKey(string)
  | IdKey(string);

type style_rule = (ast_reference, ast_reference);
type style_rule_set = (style_key, list(style_rule));

type declaration =
  | ConstDecl(string, ast_expression)
  | StateDecl(string, list(ast_property), list(ast_state_prop))
  | ViewDecl(
      string,
      option(ctxl_promise(string)),
      list(ctxl_promise(string)),
      list(ast_property),
      list(ast_expression),
    )
  | FunctionDecl(string, list(ast_property), list(ast_expression))
  | StyleDecl(string, list(ast_property), list(style_rule_set))
and ast_declaration = ctxl_promise(declaration);

type statement =
  | Import(string, list(ast_import_target))
  | Declaration(ast_declaration)
  | Main(ast_declaration);

type module_ =
  | Module(list(statement))
and ast_module = ctxl_promise(module_);
