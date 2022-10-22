open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;

let type_ = `Nil;

let parse: Grammar.Kore.primitive_parser_t =
  Keyword.nil
  >|= Node.map(_ => AST.Raw.nil)
  >|= Node.add_type(Type.Raw.(type_));

let pp: Fmt.t(unit) = (ppf, ()) => Constants.Keyword.nil |> Fmt.string(ppf);

let dump = pp;
