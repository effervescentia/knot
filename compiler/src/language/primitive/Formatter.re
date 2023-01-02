open Kore;
open AST;

let format =
  Primitive.(
    ppf =>
      fun
      | Nil => KNil.format(ppf, ())
      | Boolean(x) => KBoolean.format(ppf, x)
      | Float(float, precision) => KFloat.format(ppf, (float, precision))
      | Integer(x) => KInteger.format(ppf, x)
      | String(x) => KString.format(ppf, x)
  );
