open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

let integer: number_parser_t =
  many1(Matchers.digit)
  >|= Input.join
  >|= Node.map(Int64.of_string % AST.Raw.of_int)
  |> Matchers.lexeme;
