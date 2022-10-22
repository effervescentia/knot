open Knot.Kore;
open Parse.Onyx;

let parse: Grammar.Kore.primitive_parser_t =
  choice([Parser.float, Parser.integer]) >|= Node.map(AST.Raw.of_num);

let pp: Fmt.t(AST.number_t) =
  ppf =>
    fun
    | Integer(int) => Fmt.int64(ppf, int)
    | Float(float, precision) => Fmt.pf(ppf, "%.*f", precision, float);
