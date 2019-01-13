open Core;

module Op = Operator;

let variable = M.identifier ==> (s => Variable(s));
let dot_access = chainl1(variable, Op.dot);

let rec expr =
  dot_access
  <|> variable
  >>= (
    x =>
      M.comma_separated(Expression.expr)
      |> M.parentheses
      ==> (exprs => Execution(x, exprs))
      <|> return(x)
  );
