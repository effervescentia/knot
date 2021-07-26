open Kore;
open AST.Raw;

let _unary_op = (ctx, f, (_, cursor) as expr) => (f(expr), cursor);

let _binary_op = (ctx, f, ((_, l_cursor) as l, (_, r_cursor) as r)) => (
  (l, r) |> f,
  Cursor.join(l_cursor, r_cursor),
);

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

/* inverting (+, -) */

let negative = (ctx: ClosureContext.t) =>
  of_neg_op |> _unary_op(ctx) <$ Symbol.negative;
let positive = (ctx: ClosureContext.t) =>
  of_pos_op |> _unary_op(ctx) <$ Symbol.positive;

/* simple arithmetic (*, +, -) */

let mult = (ctx: ClosureContext.t) =>
  of_mult_op |> _binary_op(ctx) <$ Symbol.multiply;
let add = (ctx: ClosureContext.t) =>
  of_add_op |> _binary_op(ctx) <$ Symbol.add;
let sub = (ctx: ClosureContext.t) =>
  of_sub_op |> _binary_op(ctx) <$ Symbol.subtract;

/* complex arithmetic (/, ^) */

let div = (ctx: ClosureContext.t) =>
  of_div_op |> _binary_op(ctx) <$ Symbol.divide;
let expo = (ctx: ClosureContext.t) =>
  ((l, r) => (l, r) |> _binary_op(ctx, of_expo_op)) <$ Symbol.exponent;

/* logical (&&, ||) */

let logical_and = (ctx: ClosureContext.t) =>
  of_and_op |> _binary_op(ctx) <$ Glyph.logical_and;
let logical_or = (ctx: ClosureContext.t) =>
  of_or_op |> _binary_op(ctx) <$ Glyph.logical_or;

/* negating (!) */

let not = (ctx: ClosureContext.t) =>
  of_not_op |> _unary_op(ctx) <$ Symbol.not;

/* comparative (<=, <, >=, >) */

let less_or_eql = (ctx: ClosureContext.t) =>
  of_lte_op |> _binary_op(ctx) <$ Glyph.less_or_eql;
let less_than = (ctx: ClosureContext.t) =>
  of_lt_op |> _binary_op(ctx) <$ Symbol.less_than;
let greater_or_eql = (ctx: ClosureContext.t) =>
  of_gte_op |> _binary_op(ctx) <$ Glyph.greater_or_eql;
let greater_than = (ctx: ClosureContext.t) =>
  of_gt_op |> _binary_op(ctx) <$ Symbol.greater_than;

/* symmetrical (==, !=) */

let equality = (ctx: ClosureContext.t) =>
  of_eq_op |> _binary_op(ctx) <$ Glyph.equality;
let inequality = (ctx: ClosureContext.t) =>
  of_ineq_op |> _binary_op(ctx) <$ Glyph.inequality;
