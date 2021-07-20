open Kore;

let statement =
  AST.Raw.(
    fun
    | Expression((_, t, _)) => t
    | Variable(_) => K_Strong(K_Nil)
  );

let declaration =
  AST.Final.(
    fun
    | Constant((_, t, _)) => t
    | Type((t, _)) => t
    | Function(args, (_, t, _)) => Function([], t)
  );
