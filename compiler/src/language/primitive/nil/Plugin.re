open Knot.Kore;

let type_ = `Nil;

let parse = Parser.nil;

let pp: Fmt.t(unit) = (ppf, ()) => Constants.Keyword.nil |> Fmt.string(ppf);

let dump = pp;
