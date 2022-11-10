open Knot.Kore;
open AST;

module KFloat = KFloat.Plugin;
module KInteger = KInteger.Plugin;

include Framework.Primitive({
  type value_t = Primitive.number_t;

  let parse =
    Parse.Kore.(
      choice([KFloat.parse, KInteger.parse]) >|= Node.map(Raw.of_num)
    );

  let pp =
    Primitive.(
      ppf =>
        fun
        | Integer(int) => int |> KInteger.pp(ppf)
        | Float(float, precision) => (float, precision) |> KFloat.pp(ppf)
    );

  let to_xml =
    Primitive.(
      fun
      | Integer(int) => int |> KInteger.to_xml
      | Float(float, precision) => (float, precision) |> KFloat.to_xml
    );
});
