open Knot.Kore;
open AST;

let pp_closure:
  Fmt.t(Result.raw_expression_t) => Fmt.t(list(Result.statement_t)) =
  (pp_expression, ppf) =>
    fun
    | [] => Fmt.string(ppf, "{}")
    | stmts =>
      stmts
      |> List.map(fst)
      |> Fmt.(closure(KStatement.Plugin.pp(pp_expression), ppf));
