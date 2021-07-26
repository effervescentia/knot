open Kore;

let lexeme = ((_, t, _)) => t;

let statement =
  AST.(
    fun
    | Expression((_, t, _)) => t
    | Variable(_) => Valid(`Nil)
  );

let declaration =
  AST.(
    fun
    | Constant((_, t, _)) => t
    /* TODO: extract argument types */
    | Function(args, (_, t, _)) => Valid(`Function(([], t)))
  );
