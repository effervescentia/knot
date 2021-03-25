open Kore;
open AST;
open Type;

let _unary_op = (t, f, (_, type_, cursor) as expr) => (
  expr |> f,
  type_,
  cursor,
);

let _binary_op =
    (t, f, ((_, l_type, l_cursor) as l, (_, r_type, r_cursor) as r)) => (
  (l, r) |> f,
  (l_type, r_type) |> t,
  Cursor.join(l_cursor, r_cursor),
);

let _inverting =
  fun
  | K_Strong((K_Integer | K_Float) as t) => K_Strong(t)
  | t => K_Invalid(NotAssignable(t, K_Numeric));
let negative = of_neg_op |> _unary_op(_inverting) <$ Symbol.negative;
let positive = of_pos_op |> _unary_op(_inverting) <$ Symbol.positive;

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

let _simple_arithmetic =
  fun
  | (K_Strong(K_Integer), K_Strong(K_Integer)) => K_Strong(K_Integer)
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    K_Strong(K_Float)
  | (t, K_Strong(K_Integer | K_Float)) =>
    K_Invalid(NotAssignable(t, K_Numeric))
  | (_, t) => K_Invalid(NotAssignable(t, K_Numeric));
let mult = of_mult_op |> _binary_op(_simple_arithmetic) <$ Symbol.multiply;
let add = of_add_op |> _binary_op(_simple_arithmetic) <$ Symbol.add;
let sub = of_sub_op |> _binary_op(_simple_arithmetic) <$ Symbol.subtract;

let _complex_arithmetic =
  fun
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    K_Strong(K_Float)
  | (x, K_Strong(K_Integer | K_Float))
  | (_, x) => K_Invalid(NotAssignable(x, K_Numeric));
let div = of_div_op |> _binary_op(_complex_arithmetic) <$ Symbol.divide;
let expo =
  ((l, r) => (l, r) |> _binary_op(_complex_arithmetic, of_expo_op))
  <$ Symbol.exponent;

let _logical =
  fun
  | (K_Strong(K_Boolean), K_Strong(K_Boolean)) => K_Strong(K_Boolean)
  | (t, K_Strong(K_Boolean))
  | (_, t) => K_Invalid(TypeMismatch(t, K_Strong(K_Boolean)));
let not = of_not_op |> _unary_op(_logical) <$ Symbol.not;
let logical_and = of_and_op |> _binary_op(_logical) <$ Glyph.logical_and;
let logical_or = of_or_op |> _binary_op(_logical) <$ Glyph.logical_or;

let _comparative =
  fun
  | (K_Strong(K_Integer | K_Float), K_Strong(K_Integer | K_Float)) =>
    K_Strong(K_Boolean)
  | (t, K_Strong(K_Integer | K_Float))
  | (_, t) => K_Invalid(NotAssignable(t, K_Numeric));
let less_or_eql = of_lte_op |> _binary_op(_comparative) <$ Glyph.less_or_eql;
let less_than = of_lt_op |> _binary_op(_comparative) <$ Symbol.less_than;
let greater_or_eql =
  of_gte_op |> _binary_op(_comparative) <$ Glyph.greater_or_eql;
let greater_than =
  of_gt_op |> _binary_op(_comparative) <$ Symbol.greater_than;

let _symmetrical =
  fun
  | (l, r) when l == r => l
  | (l, r) => K_Invalid(TypeMismatch(l, r));
let equality = of_eq_op |> _binary_op(_symmetrical) <$ Glyph.equality;
let inequality = of_ineq_op |> _binary_op(_symmetrical) <$ Glyph.inequality;
