open Knot.Kore;
open Common;

module StyleRule = {
  type t('expr, 'typ) = (Node.t(string, 'typ), Node.t('expr, 'typ));

  type node_t('expr, 'typ) = raw_t(t('expr, 'typ));
};

module Parameter = {
  type t('expr, 'typ) = (
    identifier_t,
    option(TypeExpression.t),
    option(Node.t('expr, 'typ)),
  );

  type node_t('expr, 'typ) = Node.t(t('expr, 'typ), 'typ);
};

type t('typ) =
  | Primitive(Primitive.t)
  | Identifier(string)
  | KSX(KSX.t(t('typ), 'typ))
  | Group(node_t('typ))
  | BinaryOp(Operator.Binary.t, node_t('typ), node_t('typ))
  | UnaryOp(Operator.Unary.t, node_t('typ))
  | Closure(list(Statement.node_t(t('typ), 'typ)))
  | DotAccess(node_t('typ), identifier_t)
  | BindStyle(KSX.ViewKind.t, node_t('typ), node_t('typ))
  | FunctionCall(node_t('typ), list(node_t('typ)))
  | Style(list(StyleRule.node_t(t('typ), 'typ)))

and node_t('typ) = Node.t(t('typ), 'typ);

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
  | Style(rules) => rules |> style;
