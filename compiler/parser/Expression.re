open Core;

module Op = Operator;

let variable = M.identifier ==> (s => Variable(s));
let dot_access = chainl1(variable, Op.dot);

let rec expr = input => (chainl1(logical, Op.or_))(input)
and logical = input => (chainl1(comparative, Op.and_))(input)
and comparative = input =>
  (chainl1(add_or_sub, Op.lte <|> Op.gte <|> Op.lt <|> Op.gt))(input)
and add_or_sub = input => (chainl1(mul_or_div, Op.add <|> Op.sub))(input)
and mul_or_div = input => (chainl1(closure, Op.mul <|> Op.div))(input)
and closure = input => (M.parentheses(expr) <|> term)(input)
and reference = input =>
  (
    dot_access
    <|> variable
    >>= (
      refr =>
        M.comma_separated(expr)
        |> M.parentheses
        ==> (exprs => Execution(refr, exprs))
        |= refr
    )
  )(
    input,
  )
and term = input =>
  (
    /* JSX.expr(expr)
       ==> (j => JSX(j))
       <|>  */
    reference
    ==> (r => Reference(r))
    <|> (M.string ==> (s => StringLit(s)))
    <|> (M.number ==> (x => NumericLit(x)))
    <|> (M.boolean ==> (b => BooleanLit(b)))
  )(
    input,
  );
