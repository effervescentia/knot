open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Float = KFloat.Plugin;
module Integer = KInteger.Plugin;

type t = AST.Primitive.number_t;

let parse: primitive_parser_t =
  choice([Float.parse, Integer.parse]) >|= Node.map(AST.Raw.of_num);

let pp: Fmt.t(t) =
  ppf =>
    fun
    | Integer(int) => int |> Integer.pp(ppf)
    | Float(float, precision) => (float, precision) |> Float.pp(ppf);

let to_xml: t => Fmt.xml_t(string) =
  fun
  | Integer(int) => Integer.to_xml(int)
  | Float(float, precision) => (float, precision) |> Float.to_xml;
