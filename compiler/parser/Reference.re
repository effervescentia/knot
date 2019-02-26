open Core;

module Op = Operator;

let variable = M.identifier ==> (s => Variable(s));
let dot_access = chainl1(variable, Op.dot);

let refr = x =>
  dot_access
  <|> variable
  >>= (
    refr =>
      M.comma_separated(x)
      |> M.parentheses
      ==> List.map(no_ctx)
      % (exprs => Execution(no_ctx(refr), exprs))
      |= refr
  );
