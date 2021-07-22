open Kore;
open AST;

type type_result = result(Type2.Raw.t, Type2.Raw.error_t);
type unary_op_resolver = Type2.Raw.t => type_result;
type binary_op_resolver =
  (Tuple.t2(Cursor.t), Tuple.t2(Type2.Raw.t)) =>
  result(Type2.Raw.t, (Type2.Raw.error_t, Cursor.t));

let _report_invalid =
    (ctx: Context.t, cursor: Cursor.t, err: Type2.Raw.error_t): Type2.Raw.t => {
  ctx.report(ParseError(TypeError(err), ctx.namespace, cursor));
  `Invalid(err);
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

let _inverting: unary_op_resolver =
  fun
  | `Strong((`Integer | `Float) as t) => Ok(`Strong(t))
  | `Strong(t) => Error(NotAssignable(t, Number))
  /* TODO: support weak type assignment */
  | _ => Error(TypeResolutionFailed);

let negative = (ctx: Context.t) =>
  of_neg_op |> _unary_op(ctx, _inverting) <$ Symbol.negative;
let positive = (ctx: Context.t) =>
  of_pos_op |> _unary_op(ctx, _inverting) <$ Symbol.positive;

/* simple arithmetic (*, +, -) */

let _simple_arithmetic: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* arithmetic with real numbers */
    | (`Strong(`Integer), `Strong(`Integer)) => Ok(`Strong(`Integer))
    /* arithmetic with complex numbers */
    | (`Strong(`Integer | `Float), `Strong(`Integer | `Float)) =>
      Ok(`Strong(`Float))
    /* check arguments from right-to-left */
    | (`Strong(`Integer | `Float), `Strong(t)) =>
      Error((NotAssignable(t, Number), r_cursor))
    | (`Strong(t), _) => Error((NotAssignable(t, Number), l_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let mult = (ctx: Context.t) =>
  of_mult_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.multiply;
let add = (ctx: Context.t) =>
  of_add_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.add;
let sub = (ctx: Context.t) =>
  of_sub_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.subtract;

/* complex arithmetic (/, ^) */

let _complex_arithmetic: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* all numeric operations should result in a float */
    | (`Strong(`Integer | `Float), `Strong(`Integer | `Float)) =>
      Ok(`Strong(`Float))
    /* check arguments from right-to-left */
    | (`Strong(`Integer | `Float), `Strong(t)) =>
      Error((NotAssignable(t, Number), r_cursor))
    | (`Strong(t), _) => Error((NotAssignable(t, Number), l_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let div = (ctx: Context.t) =>
  of_div_op |> _binary_op(ctx, _complex_arithmetic) <$ Symbol.divide;
let expo = (ctx: Context.t) =>
  ((l, r) => (l, r) |> _binary_op(ctx, _complex_arithmetic, of_expo_op))
  <$ Symbol.exponent;

/* logical (&&, ||) */

let _logical: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* only allow boolean values on either side */
    | (`Strong(`Boolean), `Strong(`Boolean)) => Ok(`Strong(`Boolean))
    /* check arguments from right-to-left */
    | (`Strong(`Boolean), `Strong(t)) =>
      Error((TypeMismatch(`Boolean, t), r_cursor))
    | (`Strong(t), _) => Error((TypeMismatch(`Boolean, t), l_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let logical_and = (ctx: Context.t) =>
  of_and_op |> _binary_op(ctx, _logical) <$ Glyph.logical_and;
let logical_or = (ctx: Context.t) =>
  of_or_op |> _binary_op(ctx, _logical) <$ Glyph.logical_or;

/* negating (!) */

let _negating: unary_op_resolver =
  fun
  | `Strong(`Boolean) => Ok(`Strong(`Boolean))
  | `Strong(t) => Error(TypeMismatch(`Boolean, t))
  /* TODO: support weak type assignment */
  | _ => Error(TypeResolutionFailed);

let not = (ctx: Context.t) =>
  of_not_op |> _unary_op(ctx, _negating) <$ Symbol.not;

/* comparative (<=, <, >=, >) */

let _comparative: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* all numeric types can be compared */
    | (`Strong(`Integer | `Float), `Strong(`Integer | `Float)) =>
      Ok(`Strong(`Boolean))
    /* check arguments from right-to-left */
    | (`Strong(`Integer | `Float), `Strong(t)) =>
      Error((NotAssignable(t, Number), r_cursor))
    | (`Strong(t), _) => Error((NotAssignable(t, Number), l_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let less_or_eql = (ctx: Context.t) =>
  of_lte_op |> _binary_op(ctx, _comparative) <$ Glyph.less_or_eql;
let less_than = (ctx: Context.t) =>
  of_lt_op |> _binary_op(ctx, _comparative) <$ Symbol.less_than;
let greater_or_eql = (ctx: Context.t) =>
  of_gte_op |> _binary_op(ctx, _comparative) <$ Glyph.greater_or_eql;
let greater_than = (ctx: Context.t) =>
  of_gt_op |> _binary_op(ctx, _comparative) <$ Symbol.greater_than;

/* symmetrical (==, !=) */

let _symmetrical: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* allow comparing numeric types with each other */
    | (`Strong(`Integer | `Float), `Strong(`Integer | `Float)) =>
      Ok(`Strong(`Boolean))
    /* otherwise operands must be of the same type */
    | (l, r) when l == r => Ok(`Strong(`Boolean))
    | (`Strong(l), `Strong(r)) => Error((TypeMismatch(l, r), r_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let equality = (ctx: Context.t) =>
  of_eq_op |> _binary_op(ctx, _symmetrical) <$ Glyph.equality;
let inequality = (ctx: Context.t) =>
  of_ineq_op |> _binary_op(ctx, _symmetrical) <$ Glyph.inequality;
