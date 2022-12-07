open Knot.Kore;
open AST;

let format =
  Primitive.(
    ppf =>
      fun
      | Nil => KNil.Plugin.format(ppf, ())
      | Boolean(x) => KBoolean.Plugin.format(ppf, x)
      | Number(x) => KNumber.Plugin.format(ppf, x)
      | String(x) => KString.Plugin.format(ppf, x)
  );
