open Kore;
open AST;

let to_xml =
  Primitive.(
    fun
    | Nil => () |> KNil.to_xml
    | Boolean(boolean) => boolean |> KBoolean.to_xml
    | Integer(integer) => integer |> KInteger.to_xml
    | Float(float, precision) => (float, precision) |> KFloat.to_xml
    | String(string) => string |> KString.to_xml
  );
