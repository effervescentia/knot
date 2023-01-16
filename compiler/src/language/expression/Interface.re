open Knot.Kore;
open AST.Common;
open AST.Operator;

type t('typ) =
  | Primitive(KPrimitive.Plugin.value_t(t('typ), 'typ))
  | Identifier(KIdentifier.Plugin.value_t(t('typ), 'typ))
  | KSX(KSX.Plugin.value_t(t('typ), 'typ))
  | Group(KGroup.Plugin.value_t(t('typ), 'typ))
  | BinaryOp(KBinaryOperator.Plugin.value_t(t('typ), 'typ))
  | UnaryOp(KUnaryOperator.Plugin.value_t(t('typ), 'typ))
  | Closure(KClosure.Plugin.value_t(t('typ), 'typ))
  | DotAccess(KDotAccess.Plugin.value_t(t('typ), 'typ))
  | BindStyle(KBindStyle.Plugin.value_t(t('typ), 'typ))
  | FunctionCall(KFunctionCall.Plugin.value_t(t('typ), 'typ))
  | Style(KStyle.Plugin.value_t(t('typ), 'typ))

and node_t('typ) = Node.t(t('typ), 'typ);

/* plugin types */

module Plugin = {
  include AST.Framework.DebugPlugin.Make({
    type debug_arg_t('expr, 'typ) = 'typ => string;
    type value_t('expr, 'typ) = node_t('typ);
  });
};

/* static */

let of_primitive = x => Primitive(x);
let of_identifier = x => Identifier(x);
let of_ksx = x => KSX(x);
let of_group = x => Group(x);
let of_closure = x => Closure(x);
let of_dot_access = x => DotAccess(x);
let of_bind_style = x => BindStyle(x);
let of_function_call = x => FunctionCall(x);
let of_style = x => Style(x);
let of_binary_op = x => BinaryOp(x);
let of_unary_op = x => UnaryOp(x);

/* binary operations */

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

/* unary operations */

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
  | Primitive(x) => primitive(x)
  | Identifier(x) => identifier(x)
  | KSX(x) => ksx(x)
  | Group(x) => group(x)
  | BinaryOp(x) => binary_op(x)
  | UnaryOp(x) => unary_op(x)
  | Closure(x) => closure(x)
  | DotAccess(x) => dot_access(x)
  | BindStyle(x) => bind_style(x)
  | FunctionCall(x) => function_call(x)
  | Style(x) => style(x);
