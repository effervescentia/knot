open Kore;
open AST.Raw;

type type_result = result(Type.Raw.t, Type.Raw.error_t);
type unary_op_resolver = Type.Raw.t => type_result;
type binary_op_resolver =
  (Tuple.t2(Cursor.t), Tuple.t2(Type.Raw.t)) =>
  result(Type.Raw.t, (Type.Raw.error_t, Cursor.t));

let _report_invalid =
    (ctx: ClosureContext.t, cursor: Cursor.t, err: Type.Raw.error_t)
    : Type.Raw.t => {
  ParseError(TypeError(err), ctx.namespace_context.namespace, cursor)
  |> ClosureContext.report(ctx);
  Invalid(err);
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
  | Strong((`Integer | `Float) as t) => Ok(Strong(t))
  | Strong(_) as t => Error(NotAssignable(t, Number))
  /* TODO: support weak type assignment */
  | _ => Error(TypeResolutionFailed);

let negative = (ctx: ClosureContext.t) =>
  of_neg_op |> _unary_op(ctx, _inverting) <$ Symbol.negative;
let positive = (ctx: ClosureContext.t) =>
  of_pos_op |> _unary_op(ctx, _inverting) <$ Symbol.positive;

/* simple arithmetic (*, +, -) */

let _simple_arithmetic: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* arithmetic with real numbers */
    | (Strong(`Integer), Strong(`Integer)) => Ok(Strong(`Integer))
    /* arithmetic with complex numbers */
    | (Strong(`Integer | `Float), Strong(`Integer | `Float)) =>
      Ok(Strong(`Float))
    /* check arguments from right-to-left */
    | (Strong(`Integer | `Float), Strong(_) as t) =>
      Error((NotAssignable(t, Number), r_cursor))
    | (Strong(_) as t, _) => Error((NotAssignable(t, Number), l_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let mult = (ctx: ClosureContext.t) =>
  of_mult_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.multiply;
let add = (ctx: ClosureContext.t) =>
  of_add_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.add;
let sub = (ctx: ClosureContext.t) =>
  of_sub_op |> _binary_op(ctx, _simple_arithmetic) <$ Symbol.subtract;

/* complex arithmetic (/, ^) */

let _complex_arithmetic: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* all numeric operations should result in a float */
    | (Strong(`Integer | `Float), Strong(`Integer | `Float)) =>
      Ok(Strong(`Float))
    /* check arguments from right-to-left */
    | (Strong(`Integer | `Float), Strong(_) as t) =>
      Error((NotAssignable(t, Number), r_cursor))
    | (Strong(_) as t, _) => Error((NotAssignable(t, Number), l_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let div = (ctx: ClosureContext.t) =>
  of_div_op |> _binary_op(ctx, _complex_arithmetic) <$ Symbol.divide;
let expo = (ctx: ClosureContext.t) =>
  ((l, r) => (l, r) |> _binary_op(ctx, _complex_arithmetic, of_expo_op))
  <$ Symbol.exponent;

/* logical (&&, ||) */

let _logical: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* only allow boolean values on either side */
    | (Strong(`Boolean), Strong(`Boolean)) => Ok(Strong(`Boolean))
    /* check arguments from right-to-left */
    | (Strong(`Boolean), Strong(_) as t) =>
      Error((TypeMismatch(Strong(`Boolean), t), r_cursor))
    | (Strong(_) as t, _) =>
      Error((TypeMismatch(Strong(`Boolean), t), l_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let logical_and = (ctx: ClosureContext.t) =>
  of_and_op |> _binary_op(ctx, _logical) <$ Glyph.logical_and;
let logical_or = (ctx: ClosureContext.t) =>
  of_or_op |> _binary_op(ctx, _logical) <$ Glyph.logical_or;

/* negating (!) */

let _negating: unary_op_resolver =
  fun
  | Strong(`Boolean) => Ok(Strong(`Boolean))
  | Strong(_) as t => Error(TypeMismatch(Strong(`Boolean), t))
  /* TODO: support weak type assignment */
  | _ => Error(TypeResolutionFailed);

let not = (ctx: ClosureContext.t) =>
  of_not_op |> _unary_op(ctx, _negating) <$ Symbol.not;

/* comparative (<=, <, >=, >) */

let _comparative: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* all numeric types can be compared */
    | (Strong(`Integer | `Float), Strong(`Integer | `Float)) =>
      Ok(Strong(`Boolean))
    /* check arguments from right-to-left */
    | (Strong(`Integer | `Float), Strong(_) as t) =>
      Error((NotAssignable(t, Number), r_cursor))
    | (Strong(_) as t, _) => Error((NotAssignable(t, Number), l_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let less_or_eql = (ctx: ClosureContext.t) =>
  of_lte_op |> _binary_op(ctx, _comparative) <$ Glyph.less_or_eql;
let less_than = (ctx: ClosureContext.t) =>
  of_lt_op |> _binary_op(ctx, _comparative) <$ Symbol.less_than;
let greater_or_eql = (ctx: ClosureContext.t) =>
  of_gte_op |> _binary_op(ctx, _comparative) <$ Glyph.greater_or_eql;
let greater_than = (ctx: ClosureContext.t) =>
  of_gt_op |> _binary_op(ctx, _comparative) <$ Symbol.greater_than;

/* symmetrical (==, !=) */

let _symmetrical: binary_op_resolver =
  ((l_cursor, r_cursor)) =>
    fun
    /* allow comparing numeric types with each other */
    | (Strong(`Integer | `Float), Strong(`Integer | `Float)) =>
      Ok(Strong(`Boolean))
    /* otherwise operands must be of the same type */
    | (l, r) when l == r => Ok(Strong(`Boolean))
    | (Strong(_) as l, Strong(_) as r) =>
      Error((TypeMismatch(l, r), r_cursor))
    /* TODO: support weak type assignment */
    | _ => Error((TypeResolutionFailed, Cursor.join(l_cursor, r_cursor)));

let equality = (ctx: ClosureContext.t) =>
  of_eq_op |> _binary_op(ctx, _symmetrical) <$ Glyph.equality;
let inequality = (ctx: ClosureContext.t) =>
  of_ineq_op |> _binary_op(ctx, _symmetrical) <$ Glyph.inequality;
