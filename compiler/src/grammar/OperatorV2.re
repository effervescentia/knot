open Kore;
open AST.Raw;

let _unary_op = (ctx, f, expr) => (f(expr), Node.Raw.get_range(expr));
let _unary_op_v2 = (ctx, ~type_: option(TR.t)=?, f, x) =>
  N.(create(f(x), type_ |?: get_type(x), get_range(x)));

let _binary_op = (ctx, f, (l, r)) => (
  (l, r) |> f,
  Node.Raw.(Range.join(get_range(l), get_range(r))),
);
let _binary_op_v2 = (ctx, ~get_type=_ => TR.Valid(`Unknown), f, (l, r)) =>
  N.create((l, r) |> f, (l, r) |> get_type, N.join_ranges(l, r));

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

/* inverting (+, -) */

let negative = (ctx: ModuleContext.t) =>
  AR.of_neg_op |> _unary_op_v2(ctx) <$ Symbol.negative;
let positive = (ctx: ModuleContext.t) =>
  AR.of_pos_op |> _unary_op_v2(ctx) <$ Symbol.positive;

/* simple arithmetic (*, +, -) */

let mult = (ctx: ModuleContext.t) =>
  AR.of_mult_op
  |> _binary_op_v2(ctx, ~get_type=fst % N.get_type)
  <$ Symbol.multiply;
let add = (ctx: ModuleContext.t) =>
  AR.of_add_op |> _binary_op_v2(ctx, ~get_type=fst % N.get_type) <$ Symbol.add;
let sub = (ctx: ModuleContext.t) =>
  AR.of_sub_op
  |> _binary_op_v2(ctx, ~get_type=fst % N.get_type)
  <$ Symbol.subtract;

/* complex arithmetic (/, ^) */

let div = (ctx: ModuleContext.t) =>
  AR.of_div_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Float))
  <$ Symbol.divide;
let expo = (ctx: ModuleContext.t) =>
  (
    (l, r) =>
      (l, r)
      |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Float), AR.of_expo_op)
  )
  <$ Symbol.exponent;

/* logical (&&, ||) */

let logical_and = (ctx: ModuleContext.t) =>
  AR.of_and_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Boolean))
  <$ Glyph.logical_and;
let logical_or = (ctx: ModuleContext.t) =>
  AR.of_or_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Boolean))
  <$ Glyph.logical_or;

/* negating (!) */

let not = (ctx: ModuleContext.t) =>
  AR.of_not_op |> _unary_op_v2(ctx, ~type_=Valid(`Boolean)) <$ Symbol.not;

/* comparative (<=, <, >=, >) */

let less_or_eql = (ctx: ModuleContext.t) =>
  AR.of_lte_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Boolean))
  <$ Glyph.less_or_eql;
let less_than = (ctx: ModuleContext.t) =>
  AR.of_lt_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Boolean))
  <$ Symbol.less_than;
let greater_or_eql = (ctx: ModuleContext.t) =>
  AR.of_gte_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Boolean))
  <$ Glyph.greater_or_eql;
let greater_than = (ctx: ModuleContext.t) =>
  AR.of_gt_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Boolean))
  <$ Symbol.greater_than;

/* symmetrical (==, !=) */

let equality = (ctx: ModuleContext.t) =>
  AR.of_eq_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Boolean))
  <$ Glyph.equality;
let inequality = (ctx: ModuleContext.t) =>
  AR.of_ineq_op
  |> _binary_op_v2(ctx, ~get_type=_ => Valid(`Boolean))
  <$ Glyph.inequality;
