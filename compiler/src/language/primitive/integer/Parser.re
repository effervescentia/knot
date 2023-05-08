open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t =
  many1(Matchers.digit)
  >|= Input.join
  >|= Node.map(Int64.of_string)
  |> Matchers.lexeme;
