open Kore;

let lexeme = ((_, t, _)) => t;

let raw_statement =
  AST.Raw.(
    fun
    | Expression((_, t, _)) => t
    | Variable(_) => Strong(`Nil)
  );

let statement =
  AST.(
    fun
    | Expression((_, t, _)) => t
    | Variable(_) => Valid(`Nil)
  );

let declaration =
  AST.(
    fun
    | Constant((_, t, _)) => Type2.Result.to_raw(t)
    /* TODO: extract argument types */
    | Function(args, (_, t, _)) =>
      Strong(`Function(([], Type2.Result.to_raw(t))))
  );
