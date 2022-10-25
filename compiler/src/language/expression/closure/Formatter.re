open Knot.Kore;

let pp_closure: Fmt.t(AST.raw_statement_t) => Fmt.t(list(AST.statement_t)) =
  (pp_statement, ppf) =>
    fun
    | [] => Fmt.string(ppf, "{}")
    | stmts => stmts |> List.map(fst) |> Fmt.(closure(pp_statement, ppf));
