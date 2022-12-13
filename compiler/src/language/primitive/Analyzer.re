open Knot.Kore;
open AST;

let analyze =
  Primitive.(
    Type.(
      fun
      | Nil => Valid(`Nil)
      | Boolean(_) => Valid(`Boolean)
      | Number(Integer(_)) => Valid(`Integer)
      | Number(Float(_)) => Valid(`Float)
      | String(_) => Valid(`String)
    )
  );
