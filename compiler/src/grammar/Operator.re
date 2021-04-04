open Kore;
open AST;
open Type;

let _unary_op = (t, f, (_, type_, cursor) as expr) => (
  expr |> f,
  type_ |> t(cursor),
  cursor,
);

let _binary_op =
    (t, f, ((_, l_type, l_cursor) as l, (_, r_type, r_cursor) as r)) => {
  let cursor = Cursor.join(l_cursor, r_cursor);

  ((l, r) |> f, (l_type, r_type) |> t(cursor), cursor);
};

let _report_invalid = (ctx: Context.t, cursor, err) => {
  ctx.report(ParseError(TypeError(err), ctx.namespace, cursor));
  K_Invalid(err);
};

let _inverting = (ctx, cursor) =>
  fun
  | K_Strong((K_Integer | K_Float) as t) => K_Strong(t)
  | t => NotAssignable(t, K_Numeric) |> _report_invalid(ctx, cursor);
let negative = (ctx: Context.t) =>
  of_neg_op |> _unary_op(_inverting(ctx)) <$ Symbol.negative;
let positive = (ctx: Context.t) =>
  of_pos_op |> _unary_op(_inverting(ctx)) <$ Symbol.positive;

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

let _simple_arithmetic = (ctx, cursor) =>
  fun
  | (K_Strong(K_Integer), K_Strong(K_Integer)) => K_Strong(K_Integer)
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    K_Strong(K_Float)
  | (t, K_Strong(K_Integer | K_Float)) =>
    NotAssignable(t, K_Numeric) |> _report_invalid(ctx, cursor)
  | (_, t) => NotAssignable(t, K_Numeric) |> _report_invalid(ctx, cursor);
let mult = (ctx: Context.t) =>
  of_mult_op |> _binary_op(_simple_arithmetic(ctx)) <$ Symbol.multiply;
let add = (ctx: Context.t) =>
  of_add_op |> _binary_op(_simple_arithmetic(ctx)) <$ Symbol.add;
let sub = (ctx: Context.t) =>
  of_sub_op |> _binary_op(_simple_arithmetic(ctx)) <$ Symbol.subtract;

let _complex_arithmetic = (ctx, cursor) =>
  fun
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    K_Strong(K_Float)
  | (x, K_Strong(K_Integer | K_Float))
  | (_, x) => NotAssignable(x, K_Numeric) |> _report_invalid(ctx, cursor);
let div = (ctx: Context.t) =>
  of_div_op |> _binary_op(_complex_arithmetic(ctx)) <$ Symbol.divide;
let expo = (ctx: Context.t) =>
  ((l, r) => (l, r) |> _binary_op(_complex_arithmetic(ctx), of_expo_op))
  <$ Symbol.exponent;

let _logical = (ctx, cursor) =>
  fun
  | (K_Strong(K_Boolean), K_Strong(K_Boolean)) => K_Strong(K_Boolean)
  | (t, K_Strong(K_Boolean))
  | (_, t) =>
    TypeMismatch(t, K_Strong(K_Boolean)) |> _report_invalid(ctx, cursor);
let logical_and = (ctx: Context.t) =>
  of_and_op |> _binary_op(_logical(ctx)) <$ Glyph.logical_and;
let logical_or = (ctx: Context.t) =>
  of_or_op |> _binary_op(_logical(ctx)) <$ Glyph.logical_or;

let _negating = (ctx, cursor) =>
  fun
  | K_Strong(K_Boolean) => K_Strong(K_Boolean)
  | t =>
    TypeMismatch(t, K_Strong(K_Boolean)) |> _report_invalid(ctx, cursor);
let not = (ctx: Context.t) =>
  of_not_op |> _unary_op(_negating(ctx)) <$ Symbol.not;

let _comparative = (ctx, cursor) =>
  fun
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    K_Strong(K_Boolean)
  | (t, K_Strong(K_Integer | K_Float))
  | (_, t) => NotAssignable(t, K_Numeric) |> _report_invalid(ctx, cursor);
let less_or_eql = (ctx: Context.t) =>
  of_lte_op |> _binary_op(_comparative(ctx)) <$ Glyph.less_or_eql;
let less_than = (ctx: Context.t) =>
  of_lt_op |> _binary_op(_comparative(ctx)) <$ Symbol.less_than;
let greater_or_eql = (ctx: Context.t) =>
  of_gte_op |> _binary_op(_comparative(ctx)) <$ Glyph.greater_or_eql;
let greater_than = (ctx: Context.t) =>
  of_gt_op |> _binary_op(_comparative(ctx)) <$ Symbol.greater_than;

let _symmetrical = (ctx, cursor) =>
  fun
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    K_Strong(K_Boolean)
  | (l, r) when l == r => K_Strong(K_Boolean)
  | (l, r) => TypeMismatch(l, r) |> _report_invalid(ctx, cursor);
let equality = (ctx: Context.t) =>
  of_eq_op |> _binary_op(_symmetrical(ctx)) <$ Glyph.equality;
let inequality = (ctx: Context.t) =>
  of_ineq_op |> _binary_op(_symmetrical(ctx)) <$ Glyph.inequality;
