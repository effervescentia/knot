let parse = Parser.float;

let pp: Fmt.t((float, int)) =
  (ppf, (float, precision)) => Fmt.pf(ppf, "%.*f", precision, float);
