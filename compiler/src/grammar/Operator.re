open Kore;
open AST;

let _report_invalid =
    (
      ctx: Context.t,
      cursor: Cursor.t,
      err: Type2.Error.t(Type2.Raw.strong_t),
    )
    : Type2.Raw.t => {
  ctx.report(ParseError(TypeError(err), ctx.namespace, cursor));
  `Invalid(err);
};

let _unary_op = (ctx, t, f, (_, type_, cursor) as expr) => (
  expr |> f,
  switch (type_ |> t) {
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
  | `Strong((`Integer | `Float) as t) => Ok(`Strong(t))
  | t => Error(Type2.Error.NotAssignable(t, Number));

let negative = (ctx: Context.t) =>
  of_neg_op |> _unary_op(ctx, _inverting) <$ Symbol.negative;
let positive = (ctx: Context.t) =>
  of_pos_op |> _unary_op(ctx, _inverting) <$ Symbol.positive;

/* simple arithmetic (*, +, -) */

let _simple_arithmetic = ((l_cursor, r_cursor)) =>
  fun
  /* arithmetic with real numbers */
  | (`Strong(`Integer), `Strong(`Integer)) => Ok(`Strong(`Integer))
  /* arithmetic with complex numbers */
  | (`Strong(`Integer | `Float), `Strong(`Integer | `Float)) =>
    Ok(`Strong(`Float))
  /* check arguments from right-to-left */
  | (`Strong(`Integer | `Float), t) =>
    Error((Type2.Error.NotAssignable(t, Number), r_cursor))
  | (t, _) => Error((Type2.Error.NotAssignable(t, Number), l_cursor));

let mult = (ctx: Context.t) =>
  of_mult_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.multiply;
let add = (ctx: Context.t) =>
  of_add_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.add;
let sub = (ctx: Context.t) =>
  of_sub_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.subtract;

/* complex arithmetic (/, ^) */

let _complex_arithmetic = ((l_cursor, r_cursor)) =>
  fun
  /* all numeric operations should result in a float */
  | (`Strong(`Integer | `Float), `Strong(`Integer | `Float)) =>
    Ok(`Strong(`Float))
  /* check arguments from right-to-left */
  | (`Strong(`Integer | `Float), t) =>
    Error((Type2.Error.NotAssignable(t, K_Numeric), r_cursor))
  | (t, _) => Error((Type2.Error.NotAssignable(t, K_Numeric), l_cursor));

let div = (ctx: Context.t) =>
  of_div_op |> _binary_op(ctx, _complex_arithmetic) <$ Symbol.divide;
let expo = (ctx: Context.t) =>
  ((l, r) => (l, r) |> _binary_op(ctx, _complex_arithmetic, of_expo_op))
  <$ Symbol.exponent;

/* logical (&&, ||) */

let _logical = ((l_cursor, r_cursor)) =>
  fun
  /* only allow boolean values on either side */
  | (`Strong(`Boolean), `Strong(`Boolean)) => Ok(`Strong(`Boolean))
  /* check arguments from right-to-left */
  | (`Strong(`Boolean), t) =>
    Error((Type2.Error.TypeMismatch(`Strong(`Boolean), t), r_cursor))
  | (t, _) =>
    Error((Type2.Error.TypeMismatch(`Strong(`Boolean), t), l_cursor));

let logical_and = (ctx: Context.t) =>
  of_and_op |> _binary_op(ctx, _logical) <$ Glyph.logical_and;
let logical_or = (ctx: Context.t) =>
  of_or_op |> _binary_op(ctx, _logical) <$ Glyph.logical_or;

/* negating (!) */

let _negating =
  fun
  | `Strong(`Boolean) => Ok(`Strong(`Boolean))
  | t => Error(Type2.Error.TypeMismatch(`Strong(`Boolean), t));

let not = (ctx: Context.t) =>
  of_not_op |> _unary_op(ctx, _negating) <$ Symbol.not;

/* comparative (<=, <, >=, >) */

let _comparative = ((l_cursor, r_cursor)) =>
  fun
  /* all numeric types can be compared */
  | (`Strong(`Integer | `Float), `Strong(`Integer | `Float)) =>
    Ok(`Strong(`Boolean))
  /* check arguments from right-to-left */
  | (`Strong(`Integer | `Float), t) =>
    Error((NotAssignable(t, K_Numeric), r_cursor))
  | (t, _) => Error((NotAssignable(t, K_Numeric), l_cursor));

let less_or_eql = (ctx: Context.t) =>
  of_lte_op |> _binary_op(ctx, _comparative) <$ Glyph.less_or_eql;
let less_than = (ctx: Context.t) =>
  of_lt_op |> _binary_op(ctx, _comparative) <$ Symbol.less_than;
let greater_or_eql = (ctx: Context.t) =>
  of_gte_op |> _binary_op(ctx, _comparative) <$ Glyph.greater_or_eql;
let greater_than = (ctx: Context.t) =>
  of_gt_op |> _binary_op(ctx, _comparative) <$ Symbol.greater_than;

/* symmetrical (==, !=) */

let _symmetrical = ((_, r_cursor)) =>
  fun
  /* allow comparing numeric types with each other */
  | (`Strong(`Integer | `Float), `Strong(`Integer | `Float)) =>
    Ok(`Strong(`Boolean))
  /* otherwise operands must be of the same type */
  | (l, r) when l == r => Ok(`Strong(`Boolean))
  | (l, r) => Error((TypeMismatch(l, r), r_cursor));

let equality = (ctx: Context.t) =>
  of_eq_op |> _binary_op(ctx, _symmetrical) <$ Glyph.equality;
let inequality = (ctx: Context.t) =>
  of_ineq_op |> _binary_op(ctx, _symmetrical) <$ Glyph.inequality;
