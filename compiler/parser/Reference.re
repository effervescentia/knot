open Core;

module Op = Operator;

let variable = input => (M.identifier ==> (s => Variable(s)))(input);
let dot_access = input => chainl1(variable, Op.dot, input);

let refr = x =>
  dot_access
  <|> variable
  >>= (
    refr =>
      M.comma_separated(x)
      |> M.parentheses
      ==> (exprs => Execution(refr, exprs))
      |= refr
  );
