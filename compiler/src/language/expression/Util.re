open Kore;

let fold =
    (
      ~primitive,
      ~identifier,
      ~ksx,
      ~group,
      ~binary_op,
      ~unary_op,
      ~closure,
      ~dot_access,
      ~bind_style,
      ~function_call,
      ~style,
    ) =>
  AST.Expression.(
    fun
    | Primitive(value) => value |> primitive
    | Identifier(name) => name |> identifier
    | KSX(value) => value |> ksx
    | Group(expression) => expression |> group
    | BinaryOp(operator, lhs, rhs) => (operator, lhs, rhs) |> binary_op
    | UnaryOp(operator, expr) => (operator, expr) |> unary_op
    | Closure(statements) => statements |> closure
    | DotAccess(object_, property) => (object_, property) |> dot_access
    | BindStyle(kind, view, style) => (kind, view, style) |> bind_style
    | FunctionCall(expression, arguments) =>
      (expression, arguments) |> function_call
    | Style(rules) => rules |> style
  );
