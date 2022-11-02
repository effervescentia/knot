open Knot.Kore;

let pp_closure:
  Fmt.t(AST.Result.raw_statement_t) => Fmt.t(list(AST.Result.statement_t)) =
  (pp_statement, ppf) =>
    fun
    | [] => Fmt.string(ppf, "{}")
    | stmts => stmts |> List.map(fst) |> Fmt.(closure(pp_statement, ppf));
