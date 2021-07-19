open Kore;
open Type;
open AST.Raw.Util;

let _report_invalid = (ctx: Context.t, cursor, err) => {
  ctx.report(ParseError(TypeError(err), ctx.namespace, cursor));
  K_Invalid(err);
};

let _unary_op = (ctx, t, f, (_, type_, cursor) as expr) => (
  f(expr),
  switch (t(type_)) {
  | Ok(t) => t
  | Error(err) => _report_invalid(ctx, cursor, err)
  },
  cursor,
);

let _binary_op =
    (ctx, t, f, ((_, l_type, l_cursor) as l, (_, r_type, r_cursor) as r)) => (
  (l, r) |> f,
  switch ((l_type, r_type) |> t((l_cursor, r_cursor))) {
  | Ok(t) => t
  | Error((err, cursor)) => _report_invalid(ctx, cursor, err)
  },
  Cursor.join(l_cursor, r_cursor),
);

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

/* inverting (+, -) */

let _inverting =
  fun
  | K_Strong((K_Integer | K_Float) as t) => Ok(K_Strong(t))
  | t => Error(NotAssignable(t, K_Numeric));

let negative = (ctx: Context.t) =>
  to_neg_op |> _unary_op(ctx, _inverting) <$ Symbol.negative;
let positive = (ctx: Context.t) =>
  to_pos_op |> _unary_op(ctx, _inverting) <$ Symbol.positive;

/* simple arithmetic (*, +, -) */

let _simple_arithmetic = ((l_cursor, r_cursor)) =>
  fun
  /* arithmetic with real numbers */
  | (K_Strong(K_Integer), K_Strong(K_Integer)) => Ok(K_Strong(K_Integer))
  /* arithmetic with complex numbers */
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    Ok(K_Strong(K_Float))
  /* check arguments from right-to-left */
  | (K_Strong(K_Integer | K_Float), t) =>
    Error((NotAssignable(t, K_Numeric), r_cursor))
  | (t, _) => Error((NotAssignable(t, K_Numeric), l_cursor));

let mult = (ctx: Context.t) =>
  to_mult_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.multiply;
let add = (ctx: Context.t) =>
  to_add_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.add;
let sub = (ctx: Context.t) =>
  to_sub_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.subtract;

/* complex arithmetic (/, ^) */

let _complex_arithmetic = ((l_cursor, r_cursor)) =>
  fun
  /* all numeric operations should result in a float */
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    Ok(K_Strong(K_Float))
  /* check arguments from right-to-left */
  | (K_Strong(K_Integer | K_Float), t) =>
    Error((NotAssignable(t, K_Numeric), r_cursor))
  | (t, _) => Error((NotAssignable(t, K_Numeric), l_cursor));

let div = (ctx: Context.t) =>
  to_div_op |> _binary_op(ctx, _complex_arithmetic) <$ Symbol.divide;
let expo = (ctx: Context.t) =>
  ((l, r) => (l, r) |> _binary_op(ctx, _complex_arithmetic, to_expo_op))
  <$ Symbol.exponent;

/* logical (&&, ||) */

let _logical = ((l_cursor, r_cursor)) =>
  fun
  /* only allow boolean values on either side */
  | (K_Strong(K_Boolean), K_Strong(K_Boolean)) => Ok(K_Strong(K_Boolean))
  /* check arguments from right-to-left */
  | (K_Strong(K_Boolean), t) =>
    Error((TypeMismatch(K_Strong(K_Boolean), t), r_cursor))
  | (t, _) => Error((TypeMismatch(K_Strong(K_Boolean), t), l_cursor));

let logical_and = (ctx: Context.t) =>
  to_and_op |> _binary_op(ctx, _logical) <$ Glyph.logical_and;
let logical_or = (ctx: Context.t) =>
  to_or_op |> _binary_op(ctx, _logical) <$ Glyph.logical_or;

/* negating (!) */

let _negating =
  fun
  | K_Strong(K_Boolean) => Ok(K_Strong(K_Boolean))
  | t => Error(TypeMismatch(K_Strong(K_Boolean), t));

let not = (ctx: Context.t) =>
  to_not_op |> _unary_op(ctx, _negating) <$ Symbol.not;

/* comparative (<=, <, >=, >) */

let _comparative = ((l_cursor, r_cursor)) =>
  fun
  /* all numeric types can be compared */
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    Ok(K_Strong(K_Boolean))
  /* check arguments from right-to-left */
  | (K_Strong(K_Integer | K_Float), t) =>
    Error((NotAssignable(t, K_Numeric), r_cursor))
  | (t, _) => Error((NotAssignable(t, K_Numeric), l_cursor));

let less_or_eql = (ctx: Context.t) =>
  to_lte_op |> _binary_op(ctx, _comparative) <$ Glyph.less_or_eql;
let less_than = (ctx: Context.t) =>
  to_lt_op |> _binary_op(ctx, _comparative) <$ Symbol.less_than;
let greater_or_eql = (ctx: Context.t) =>
  to_gte_op |> _binary_op(ctx, _comparative) <$ Glyph.greater_or_eql;
let greater_than = (ctx: Context.t) =>
  to_gt_op |> _binary_op(ctx, _comparative) <$ Symbol.greater_than;

/* symmetrical (==, !=) */

let _symmetrical = ((_, r_cursor)) =>
  fun
  /* allow comparing numeric types with each other */
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    Ok(K_Strong(K_Boolean))
  /* otherwise operands must be of the same type */
  | (l, r) when l == r => Ok(K_Strong(K_Boolean))
  | (l, r) => Error((TypeMismatch(l, r), r_cursor));

let equality = (ctx: Context.t) =>
  to_eq_op |> _binary_op(ctx, _symmetrical) <$ Glyph.equality;
let inequality = (ctx: Context.t) =>
  to_ineq_op |> _binary_op(ctx, _symmetrical) <$ Glyph.inequality;
