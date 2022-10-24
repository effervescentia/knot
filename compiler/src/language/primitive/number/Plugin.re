open Knot.Kore;
open Parse.Onyx;

let parse: Grammar.Kore.primitive_parser_t =
  choice([KFloat.Plugin.parse, KInteger.Plugin.parse])
  >|= Node.map(AST.Raw.of_num);

let pp: Fmt.t(AST.number_t) =
  ppf =>
    fun
    | Integer(int) => int |> KInteger.Plugin.pp(ppf)
    | Float(float, precision) =>
      (float, precision) |> KFloat.Plugin.pp(ppf);
