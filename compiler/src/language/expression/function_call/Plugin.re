open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let parse =
    (
      parse_term: Grammar.Kore.expression_parser_t,
      parse_expr: Grammar.Kore.expression_parser_t,
    ) => {
  let rec loop = expr =>
    parse_expr
    |> Matchers.comma_sep
    |> Matchers.between(Symbol.open_group, Symbol.close_group)
    >>= (
      args =>
        loop(
          Node.typed(
            (expr, fst(args)) |> AST.Raw.of_func_call,
            switch (Node.get_type(expr)) {
            | `Function(_, result) => result
            | _ => Type.Raw.(`Unknown)
            },
            Node.get_range(args),
          ),
        )
    )
    |> option(expr);

  parse_term >>= loop;
};

let pp:
  Fmt.t(AST.raw_expression_t) =>
  Fmt.t((AST.expression_t, list(AST.expression_t))) =
  (pp_expression, ppf, ((expr, _), args)) =>
    Fmt.(
      pf(
        ppf,
        "%a@[<hv>(%a)@]",
        pp_expression,
        expr,
        list(~sep=Sep.trailing_comma, pp_expression),
        args |> List.map(fst),
      )
    );
