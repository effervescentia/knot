open Knot.Kore;

module Matchers = Parse.Matchers;

let parse = Parser.string;

let analyze = x => Node.add_type(AST.Type.Valid(`String), x);

let pp: Fmt.t(string) =
  (ppf, x) => x |> String.escaped |> Fmt.pf(ppf, "\"%s\"");

let dump = pp;
