open Kore;
open AST;

let statement =
  fun
  | Expression((_, t, _)) => t
  | Variable(_) => `Strong(`Nil);

let declaration =
  fun
  | Constant((_, t, _)) => t
  | Function(args, (_, t, _)) => `Strong(`Function(([], t)));
