open Kore;
open AST;
open Type;

let statement =
  fun
  | Expression((_, t, _)) => t
  | Variable(_) => K_Strong(K_Nil);

let declaration =
  fun
  | Constant((_, t, _)) => t;
