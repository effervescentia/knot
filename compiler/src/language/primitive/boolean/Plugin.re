open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;

let type_ = `Boolean;

let parse: Grammar.Kore.primitive_parser_t =
  Keyword.true_
  >|= Node.map(_ => AST.Raw.of_bool(true))
  >|= Node.add_type(Type.Raw.(type_))
  <|> (
    Keyword.false_
    >|= Node.map(_ => AST.Raw.of_bool(false))
    >|= Node.add_type(Type.Raw.(type_))
  );

let analyze = x => Node.add_type(Type.Valid(type_), x);

let pp: Fmt.t(bool) =
  ppf =>
    (
      fun
      | true => Constants.Keyword.true_
      | false => Constants.Keyword.false_
    )
    % Fmt.string(ppf);

let dump = pp;
