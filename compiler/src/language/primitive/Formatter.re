open Kore;
open AST;

let format =
  Primitive.(
    ppf =>
      fun
      | Nil => KNil.format(ppf, ())
      | Boolean(x) => KBoolean.format(ppf, x)
      | Number(x) => KNumber.format(ppf, x)
      | String(x) => KString.format(ppf, x)
  );
