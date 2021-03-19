open Kore;
open AST;
open Type;

let statement =
  fun
  | Expression((_, t, _)) => t
  | Variable(_) => K_Nil;
