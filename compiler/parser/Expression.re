open Core;

module Op = Operator;

let string_lit = M.string ==> (s => StringLit(s));
let numeric_lit = M.number ==> (x => NumericLit(x));
let boolean_lit = M.boolean ==> (b => BooleanLit(b));

let rec expr = input => (chainl1(logical, Op.or_))(input)
and logical = input => (chainl1(comparative, Op.and_))(input)
and comparative = input =>
  (chainl1(add_or_sub, Op.lte <|> Op.gte <|> Op.lt <|> Op.gt))(input)
and add_or_sub = input => (chainl1(mul_or_div, Op.add <|> Op.sub))(input)
and mul_or_div = input => (chainl1(closure, Op.mul <|> Op.div))(input)
and closure = input => (M.parentheses(expr) <|> term)(input)
and term = input =>
  (
    JSX.expr(expr)
    ==> (jsx => JSX(jsx))
    <|> (Reference.refr(expr) ==> no_ctx % (r => Reference(r)))
    <|> string_lit
    <|> numeric_lit
    <|> boolean_lit
  )(
    input,
  );
