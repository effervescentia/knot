open Kore;
open AST;

let to_xml =
  Primitive.(
    fun
    | Nil => KNil.to_xml()
    | Boolean(x) => KBoolean.to_xml(x)
    | Float(x, precision) => KFloat.to_xml((x, precision))
    | Integer(x) => KInteger.to_xml(x)
    | String(x) => KString.to_xml(x)
  );
