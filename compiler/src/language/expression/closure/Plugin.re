open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let parse =
    (parse_stmt: Grammar.Kore.statement_parser_t)
    : Grammar.Kore.expression_parser_t =>
  parse_stmt
  |> many
  |> Matchers.between(Symbol.open_closure, Symbol.close_closure)
  >|= (
    ((stmts, _) as stmts_node) => {
      let last_stmt = List.last(stmts);

      Node.typed(
        AST.Raw.of_closure(stmts),
        /* if the statement list is empty the return type is nil */
        last_stmt |?> Node.get_type |?: Type.Raw.(`Nil),
        Node.get_range(stmts_node),
      );
    }
  );

let pp: Fmt.t(AST.raw_statement_t) => Fmt.t(list(AST.statement_t)) =
  (pp_statement, ppf) =>
    fun
    | [] => Fmt.string(ppf, "{}")
    | stmts => stmts |> List.map(fst) |> Fmt.(closure(pp_statement, ppf));
