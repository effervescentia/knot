open Globals;

module Op = Operator;

let variable = M.identifier ==> (s => Variable(s));
let sidecar_variable = M.sidecar_identifier ==> (s => SidecarVariable(s));
let dot_access = chainl1(sidecar_variable <|> variable, Op.dot);

let refr = x =>
  dot_access
  <|> sidecar_variable
  <|> variable
  >>= (
    refr =>
      M.comma_separated(x)
      |> M.parentheses
      ==> List.map(no_ctx)
      % (exprs => Execution(no_ctx(refr), exprs))
      |= refr
  );
