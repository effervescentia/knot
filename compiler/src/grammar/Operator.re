open Kore;
open AST;
open Type;

let _logical = _ => K_Boolean;
let _inverting =
  fun
  | K_Integer => K_Integer
  | K_Float => K_Float
  | _ => K_Invalid;
let _float = _ => K_Float;
let _arithmetic =
  fun
  | (K_Integer, K_Integer) => K_Integer
  | (K_Float, K_Integer)
  | (K_Integer, K_Float)
  | (K_Float, K_Float) => K_Float
  | _ => K_Invalid;

let _unary_op = (t, f, (_, type_, cursor) as expr) => (
  expr |> f,
  type_,
  cursor,
);

let not = of_not_op |> _unary_op(_logical) <$ Symbol.not;
let negative = of_neg_op |> _unary_op(_inverting) <$ Symbol.negative;
let positive = of_pos_op |> _unary_op(_inverting) <$ Symbol.positive;

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

let _binary_op =
    (t, f, ((_, l_type, l_cursor) as l, (_, r_type, r_cursor) as r)) => (
  (l, r) |> f,
  (l_type, r_type) |> t,
  Cursor.join(l_cursor, r_cursor),
);

let mult = of_mult_op |> _binary_op(_arithmetic) <$ Symbol.multiply;
let div = of_div_op |> _binary_op(_float) <$ Symbol.divide;
let add = of_add_op |> _binary_op(_arithmetic) <$ Symbol.add;
let sub = of_sub_op |> _binary_op(_arithmetic) <$ Symbol.subtract;

let logical_and = of_and_op |> _binary_op(_logical) <$ Glyph.logical_and;
let logical_or = of_or_op |> _binary_op(_logical) <$ Glyph.logical_or;

let less_or_eql = of_lte_op |> _binary_op(_logical) <$ Glyph.less_or_eql;
let less_than = of_lt_op |> _binary_op(_logical) <$ Symbol.less_than;
let greater_or_eql =
  of_gte_op |> _binary_op(_logical) <$ Glyph.greater_or_eql;
let greater_than = of_gt_op |> _binary_op(_logical) <$ Symbol.greater_than;

let equality = of_eq_op |> _binary_op(_logical) <$ Glyph.equality;
let inequality = of_ineq_op |> _binary_op(_logical) <$ Glyph.inequality;

let expo =
  ((l, r) => (l, r) |> _binary_op(_float, of_expo_op)) <$ Symbol.exponent;
