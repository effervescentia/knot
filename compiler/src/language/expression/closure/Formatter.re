open Knot.Kore;
open AST;

let pp_closure:
  Fmt.t(Result.raw_statement_t) => Fmt.t(list(Result.statement_t)) =
  (pp_statement, ppf) =>
    fun
    | [] => Fmt.string(ppf, "{}")
    | stmts => stmts |> List.map(fst) |> Fmt.(closure(pp_statement, ppf));
