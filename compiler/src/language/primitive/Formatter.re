open Kore;
open AST;

let format =
  Primitive.(
    ppf => {
      let (&>) = (args, format) => args |> format(ppf);

      fun
      | Nil => () &> KNil.format
      | Boolean(boolean) => boolean &> KBoolean.format
      | Integer(integer) => integer &> KInteger.format
      | Float(float, precision) => (float, precision) &> KFloat.format
      | String(string) => string &> KString.format;
    }
  );
