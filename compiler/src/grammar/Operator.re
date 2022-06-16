open Kore;

let _unary_op = (ctx, ~type_: option(TR.t)=?, f, x) =>
  N2.typed(f(x), type_ |?: N2.get_type(x), N2.get_range(x));

let _binary_op = (ctx, ~get_type=_ => TR.(`Unknown), f, (l, r)) =>
  N2.typed((l, r) |> f, (l, r) |> get_type, N2.join_ranges(l, r));

let _get_arithmetic_result_type =
  TR.(
    Tuple.map2(N2.get_type)
    % (
      fun
      | (`Float, _)
      | (_, `Float) => `Float
      | (`Integer, `Integer) => `Integer

      /* cannot determine the type early in this case */
      | _ => `Unknown
    )
  );

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

/* inverting (+, -) */

let negative = (ctx: ModuleContext.t) =>
  AR.of_neg_op |> _unary_op(ctx) <$ Symbol.negative;
let positive = (ctx: ModuleContext.t) =>
  AR.of_pos_op |> _unary_op(ctx) <$ Symbol.positive;

/* simple arithmetic (*, +, -) */

let mult = (ctx: ModuleContext.t) =>
  AR.of_mult_op
  |> _binary_op(ctx, ~get_type=_get_arithmetic_result_type)
  <$ Symbol.multiply;
let add = (ctx: ModuleContext.t) =>
  AR.of_add_op
  |> _binary_op(ctx, ~get_type=_get_arithmetic_result_type)
  <$ Symbol.add;
let sub = (ctx: ModuleContext.t) =>
  AR.of_sub_op
  |> _binary_op(ctx, ~get_type=_get_arithmetic_result_type)
  <$ Symbol.subtract;

/* complex arithmetic (/, ^) */

let div = (ctx: ModuleContext.t) =>
  AR.of_div_op |> _binary_op(ctx, ~get_type=_ => `Float) <$ Symbol.divide;
let expo = (ctx: ModuleContext.t) =>
  (
    (l, r) => (l, r) |> _binary_op(ctx, ~get_type=_ => `Float, AR.of_expo_op)
  )
  <$ Symbol.exponent;

/* logical (&&, ||) */

let logical_and = (ctx: ModuleContext.t) =>
  AR.of_and_op |> _binary_op(ctx, ~get_type=_ => `Boolean) <$ Glyph.logical_and;
let logical_or = (ctx: ModuleContext.t) =>
  AR.of_or_op |> _binary_op(ctx, ~get_type=_ => `Boolean) <$ Glyph.logical_or;

/* negating (!) */

let not = (ctx: ModuleContext.t) =>
  AR.of_not_op |> _unary_op(ctx, ~type_=`Boolean) <$ Symbol.not;

/* comparative (<=, <, >=, >) */

let less_or_eql = (ctx: ModuleContext.t) =>
  AR.of_lte_op |> _binary_op(ctx, ~get_type=_ => `Boolean) <$ Glyph.less_or_eql;
let less_than = (ctx: ModuleContext.t) =>
  AR.of_lt_op |> _binary_op(ctx, ~get_type=_ => `Boolean) <$ Symbol.less_than;
let greater_or_eql = (ctx: ModuleContext.t) =>
  AR.of_gte_op
  |> _binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Glyph.greater_or_eql;
let greater_than = (ctx: ModuleContext.t) =>
  AR.of_gt_op
  |> _binary_op(ctx, ~get_type=_ => `Boolean)
  <$ Symbol.greater_than;

/* symmetrical (==, !=) */

let equality = (ctx: ModuleContext.t) =>
  AR.of_eq_op |> _binary_op(ctx, ~get_type=_ => `Boolean) <$ Glyph.equality;
let inequality = (ctx: ModuleContext.t) =>
  AR.of_ineq_op |> _binary_op(ctx, ~get_type=_ => `Boolean) <$ Glyph.inequality;
