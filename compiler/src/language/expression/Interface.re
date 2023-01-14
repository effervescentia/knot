open Knot.Kore;
open AST.Common;
open AST.Operator;

type t('typ) =
  | Primitive(KPrimitive.Interface.t)
  | Identifier(string)
  | KSX(KSX.Interface.t(t('typ), 'typ))
  | Group(node_t('typ))
  | BinaryOp(Binary.t, node_t('typ), node_t('typ))
  | UnaryOp(Unary.t, node_t('typ))
  | Closure(list(KStatement.Interface.node_t(t('typ), 'typ)))
  | DotAccess(node_t('typ), identifier_t)
  | BindStyle(KSX.Interface.ViewKind.t, node_t('typ), node_t('typ))
  | FunctionCall(node_t('typ), list(node_t('typ)))
  | Style(list(KStyle.Interface.StyleRule.node_t(t('typ), 'typ)))

and node_t('typ) = Node.t(t('typ), 'typ);

/* plugin types */

module Plugin = {
  include AST.Framework.DebugPlugin.Make({
    type debug_arg_t('expr, 'typ) = 'typ => string;
    type value_t('expr, 'typ) = node_t('typ);
  });
};

/* static */

let of_primitive = primitive => Primitive(primitive);
let of_identifier = name => Identifier(name);
let of_ksx = ksx => KSX(ksx);
let of_group = expression => Group(expression);
let of_closure = statements => Closure(statements);
let of_dot_access = ((object_, property)) => DotAccess(object_, property);
let of_bind_style = ((kind, view, style)) => BindStyle(kind, view, style);
let of_function_call = ((function_, arguments)) =>
  FunctionCall(function_, arguments);
let of_style = rules => Style(rules);

let of_binary_op = ((op, lhs, rhs)) => BinaryOp(op, lhs, rhs);
let of_or_op = ((lhs, rhs)) => (Binary.LogicalOr, lhs, rhs) |> of_binary_op;
let of_and_op = ((lhs, rhs)) =>
  (Binary.LogicalAnd, lhs, rhs) |> of_binary_op;
let of_lt_op = ((lhs, rhs)) => (Binary.LessThan, lhs, rhs) |> of_binary_op;
let of_lte_op = ((lhs, rhs)) =>
  (Binary.LessOrEqual, lhs, rhs) |> of_binary_op;
let of_gt_op = ((lhs, rhs)) =>
  (Binary.GreaterThan, lhs, rhs) |> of_binary_op;
let of_gte_op = ((lhs, rhs)) =>
  (Binary.GreaterOrEqual, lhs, rhs) |> of_binary_op;
let of_equal_op = ((lhs, rhs)) => (Binary.Equal, lhs, rhs) |> of_binary_op;
let of_unequal_op = ((lhs, rhs)) =>
  (Binary.Unequal, lhs, rhs) |> of_binary_op;
let of_add_op = ((lhs, rhs)) => (Binary.Add, lhs, rhs) |> of_binary_op;
let of_subtract_op = ((lhs, rhs)) =>
  (Binary.Subtract, lhs, rhs) |> of_binary_op;
let of_multiply_op = ((lhs, rhs)) =>
  (Binary.Multiply, lhs, rhs) |> of_binary_op;
let of_divide_op = ((lhs, rhs)) =>
  (Binary.Divide, lhs, rhs) |> of_binary_op;
let of_exponent_op = ((lhs, rhs)) =>
  (Binary.Exponent, lhs, rhs) |> of_binary_op;

let of_unary_op = ((op, expression)) => UnaryOp(op, expression);
let of_absolute_op = expression =>
  (Unary.Positive, expression) |> of_unary_op;
let of_negative_op = expression =>
  (Unary.Negative, expression) |> of_unary_op;
let of_not_op = expression => (Unary.Not, expression) |> of_unary_op;

/* methods */

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
