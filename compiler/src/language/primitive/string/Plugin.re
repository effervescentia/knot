open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;

let type_ = `String;

let parse = Parser.string;

let analyze = x => Node.add_type(Type.Valid(type_), x);

let pp: Fmt.t(string) =
  (ppf, x) => x |> String.escaped |> Fmt.pf(ppf, "\"%s\"");

let dump = pp;
