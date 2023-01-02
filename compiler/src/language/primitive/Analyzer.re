open Knot.Kore;
open AST;

let analyze =
  Primitive.(
    Type.(
      fun
      | Nil => Valid(`Nil)
      | Boolean(_) => Valid(`Boolean)
      | Integer(_) => Valid(`Integer)
      | Float(_) => Valid(`Float)
      | String(_) => Valid(`String)
    )
  );
