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
let of_binary_op = (op, (lhs, rhs)) => BinaryOp((op, lhs, rhs));
let of_unary_op = (op, x) => UnaryOp((op, x));

/* binary operations */

let of_or_op = x => of_binary_op(Binary.LogicalOr, x);
let of_and_op = x => of_binary_op(Binary.LogicalAnd, x);
let of_lt_op = x => of_binary_op(Binary.LessThan, x);
let of_lte_op = x => of_binary_op(Binary.LessOrEqual, x);
let of_gt_op = x => of_binary_op(Binary.GreaterThan, x);
let of_gte_op = x => of_binary_op(Binary.GreaterOrEqual, x);
let of_equal_op = x => of_binary_op(Binary.Equal, x);
let of_unequal_op = x => of_binary_op(Binary.Unequal, x);
let of_add_op = x => of_binary_op(Binary.Add, x);
let of_subtract_op = x => of_binary_op(Binary.Subtract, x);
let of_multiply_op = x => of_binary_op(Binary.Multiply, x);
let of_divide_op = x => of_binary_op(Binary.Divide, x);
let of_exponent_op = x => of_binary_op(Binary.Exponent, x);

/* unary operations */

let of_absolute_op = x => x |> of_unary_op(Unary.Positive);
let of_negative_op = x => x |> of_unary_op(Unary.Negative);
let of_not_op = x => x |> of_unary_op(Unary.Not);

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
