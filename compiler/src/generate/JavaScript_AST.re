open Kore;

type expression_t =
  | Null
  | Boolean(bool)
  | Number(string)
  | String(string)
  | Group(expression_t)
  | Identifier(string)
  | DotAccess(expression_t, string)
  | FunctionCall(expression_t, list(expression_t))
  | UnaryOp(string, expression_t)
  | BinaryOp(string, expression_t, expression_t)
  | Ternary(expression_t, expression_t, expression_t)
  | Function(option(string), list(string), list(statement_t))
  | Object(list((string, expression_t)))
and statement_t =
  | Expression(expression_t)
  | Variable(string, expression_t)
  | Assignment(expression_t, expression_t)
  | Return(option(expression_t))
  | DefaultImport(string, string)
  | Import(string, list((string, option(string))))
  | Export(string)
  | EmptyExport;

type program_t = list(statement_t);

let iife = statements =>
  FunctionCall(Group(Function(None, [], statements)), []);
