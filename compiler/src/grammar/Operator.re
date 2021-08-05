open Kore;
open AST.Raw;

let _unary_op = (ctx, f, expr) => (f(expr), Node.Raw.cursor(expr));

let _binary_op = (ctx, f, (l, r)) => (
  (l, r) |> f,
  Cursor.join(Node.Raw.cursor(l), Node.Raw.cursor(r)),
);

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

/* inverting (+, -) */

let negative = (ctx: ModuleContext.t) =>
  of_neg_op |> _unary_op(ctx) <$ Symbol.negative;
let positive = (ctx: ModuleContext.t) =>
  of_pos_op |> _unary_op(ctx) <$ Symbol.positive;

/* simple arithmetic (*, +, -) */

let mult = (ctx: ModuleContext.t) =>
  of_mult_op |> _binary_op(ctx) <$ Symbol.multiply;
let add = (ctx: ModuleContext.t) =>
  of_add_op |> _binary_op(ctx) <$ Symbol.add;
let sub = (ctx: ModuleContext.t) =>
  of_sub_op |> _binary_op(ctx) <$ Symbol.subtract;

/* complex arithmetic (/, ^) */

let div = (ctx: ModuleContext.t) =>
  of_div_op |> _binary_op(ctx) <$ Symbol.divide;
let expo = (ctx: ModuleContext.t) =>
  ((l, r) => (l, r) |> _binary_op(ctx, of_expo_op)) <$ Symbol.exponent;

/* logical (&&, ||) */

let logical_and = (ctx: ModuleContext.t) =>
  of_and_op |> _binary_op(ctx) <$ Glyph.logical_and;
let logical_or = (ctx: ModuleContext.t) =>
  of_or_op |> _binary_op(ctx) <$ Glyph.logical_or;

/* negating (!) */

let not = (ctx: ModuleContext.t) =>
  of_not_op |> _unary_op(ctx) <$ Symbol.not;

/* comparative (<=, <, >=, >) */

let less_or_eql = (ctx: ModuleContext.t) =>
  of_lte_op |> _binary_op(ctx) <$ Glyph.less_or_eql;
let less_than = (ctx: ModuleContext.t) =>
  of_lt_op |> _binary_op(ctx) <$ Symbol.less_than;
let greater_or_eql = (ctx: ModuleContext.t) =>
  of_gte_op |> _binary_op(ctx) <$ Glyph.greater_or_eql;
let greater_than = (ctx: ModuleContext.t) =>
  of_gt_op |> _binary_op(ctx) <$ Symbol.greater_than;

/* symmetrical (==, !=) */

let equality = (ctx: ModuleContext.t) =>
  of_eq_op |> _binary_op(ctx) <$ Glyph.equality;
let inequality = (ctx: ModuleContext.t) =>
  of_ineq_op |> _binary_op(ctx) <$ Glyph.inequality;
