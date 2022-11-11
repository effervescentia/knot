open Knot.Kore;
open AST;

module KFloat = KFloat.Plugin;
module KInteger = KInteger.Plugin;

let pp =
  Primitive.(
    ppf =>
      fun
      | Integer(int) => int |> KInteger.format(ppf)
      | Float(float, precision) => (float, precision) |> KFloat.format(ppf)
  );

include Framework.Primitive({
  type value_t = Primitive.number_t;

  let parse =
    Parse.Kore.(
      choice([KFloat.parse, KInteger.parse]) >|= Node.map(Raw.of_num)
    );

  let format = pp;

  let to_xml =
    Primitive.(
      fun
      | Integer(int) => int |> KInteger.to_xml
      | Float(float, precision) => (float, precision) |> KFloat.to_xml
    );
});
